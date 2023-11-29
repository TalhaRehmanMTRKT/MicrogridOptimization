import torch
import pandas as pd
import numpy.random as rd
import os
import numpy as np

class Arguments:
    '''revise here for our own purpose'''

    def __init__(self, agent=None, env=None):
        self.agent = agent  # Deep Reinforcement Learning algorithm
        self.env = env  # the environment for training
        self.plot_shadow_on = False  # control do we need to plot all shadow figures
        self.cwd = None  # current work directory. None means set automatically
        self.if_remove = False  # remove the cwd folder? (True, False, None:ask me)
        # self.replace_train_data=True
        self.visible_gpu = '0,1,2,3'  # for example: os.environ['CUDA_VISIBLE_DEVICES'] = '0, 2,'
        self.worker_num = 2  # rollout workers number pre GPU (adjust it to get high GPU usage)
        self.num_threads = 8  # cpu_num for evaluate model, torch.set_num_threads(self.num_threads)

        '''Arguments for training'''
        self.num_episode = 2000
        self.gamma = 0.995  # discount factor of future rewards
        # self.reward_scale = 1  # an approximate target reward usually be closed to 256
        self.learning_rate = 2 ** -14  # 2 ** -14 ~= 6e-5
        self.soft_update_tau = 2 ** -8  # 2 ** -8 ~= 5e-3

        self.net_dim = 256  # the network width 256
        self.batch_size = 4096  # num of transitions sampled from replay buffer.
        self.repeat_times = 2 ** 5  # repeatedly update network to keep critic's loss small
        self.target_step = 4096  # collect target_step experiences , then update network, 1024
        self.max_memo = 500000  # capacity of replay buffer
        self.if_per_or_gae = False  # PER for off-policy sparse reward: Prioritized Experience Replay.

        '''Arguments for evaluate'''
        # self.eval_gap = 2 ** 6  # evaluate the agent per eval_gap seconds
        # self.eval_times = 2  # number of times that get episode return in first
        self.random_seed = 0  # initialize random seed in self.init_before_training()
        self.random_seed_list = [1234, 2234, 3234, 4234, 5234]
        '''Arguments for save and plot issues'''
        self.train = True
        self.save_network = True
        self.test_network = True
        self.save_test_data = True
        self.compare_with_pyomo = True
        self.plot_on = True

    def init_before_training(self, if_main):
        if self.cwd is None:
            agent_name = self.agent.__class__.__name__
            self.cwd = f'./{agent_name}'

        if if_main:
            import shutil  # remove history according to bool(if_remove)
            if self.if_remove is None:
                self.if_remove = bool(input(f"| PRESS 'y' to REMOVE: {self.cwd}? ") == 'y')
            elif self.if_remove:
                shutil.rmtree(self.cwd, ignore_errors=True)
                print(f"| Remove cwd: {self.cwd}")
            os.makedirs(self.cwd, exist_ok=True)

        np.random.seed(self.random_seed)
        torch.manual_seed(self.random_seed)
        torch.set_num_threads(self.num_threads)
        torch.set_default_dtype(torch.float32)

        os.environ['CUDA_VISIBLE_DEVICES'] = str(self.visible_gpu)  # control how many GPU is used 　


def test_one_episode(env, act, device):
    '''to get evaluate information, here record the unblance of after taking action'''
    record_state = []
    record_action = []
    record_reward = []
    record_output = []
    record_cost = []
    record_unbalance = []
    record_system_info = []  # [time price, netload,action,real action, output*4,soc,unbalance(exchange+penalty)]

    record_init_info = []  # should include month,day,time,intial soc,initial
    env.TRAIN = False
    state = env.reset()
    record_init_info.append([env.month, env.day, env.current_time, env.battery.current_capacity])
    print(f'current testing month is {env.month}, day is {env.day},initial_soc is {env.battery.current_capacity}')
    for i in range(24):
        s_tensor = torch.as_tensor((state,), device=device)
        a_tensor = act(s_tensor)
        action = a_tensor.detach().cpu().numpy()[0]  # not need detach(), because with torch.no_grad() outside
        real_action = action
        state, next_state, reward, done = env.step(action)

        record_system_info.append(
            [state[0], state[1], state[3], action, real_action, env.battery.SOC(), env.battery.energy_change,
             next_state[4], next_state[5], next_state[6], env.unbalance, env.operation_cost])

        record_state.append(state)
        record_action.append(real_action)
        record_reward.append(reward)
        record_output.append(env.current_output)
        record_unbalance.append(env.unbalance)
        state = next_state
    record_system_info[-1][7:10] = [env.final_step_outputs[0], env.final_step_outputs[1], env.final_step_outputs[2]]
    ## add information of last step soc
    record_system_info[-1][5] = env.final_step_outputs[3]
    record = {'init_info': record_init_info, 'information': record_system_info, 'state': record_state,
              'action': record_action, 'reward': record_reward, 'cost': record_cost, 'unbalance': record_unbalance,
              'record_output': record_output}
    return record


def get_episode_return(env, act, device):
    episode_return = 0.0  # sum of rewards in an episode
    episode_unbalance = 0.0
    state = env.reset()
    for i in range(24):
        s_tensor = torch.as_tensor((state,), device=device)
        a_tensor = act(s_tensor)
        action = a_tensor.detach().cpu().numpy()[0]  # not need detach(), because with torch.no_grad() outside
        state, next_state, reward, done, = env.step(action)
        state = next_state
        episode_return += reward
        episode_unbalance += env.real_unbalance
        if done:
            break
    return episode_return, episode_unbalance


class ReplayBuffer:
    def __init__(self, max_len, state_dim, action_dim, gpu_id=0):
        self.now_len = 0
        self.next_idx = 0
        self.if_full = False
        self.max_len = max_len
        self.data_type = torch.float32
        self.action_dim = action_dim
        self.device = torch.device(f"cuda:{gpu_id}" if (torch.cuda.is_available() and (gpu_id >= 0)) else "cpu")

        other_dim = 1 + 1 + self.action_dim
        self.buf_other = torch.empty(size=(max_len, other_dim), dtype=self.data_type, device=self.device)

        if isinstance(state_dim, int):  # state is pixel
            self.buf_state = torch.empty((max_len, state_dim), dtype=torch.float32, device=self.device)
        elif isinstance(state_dim, tuple):
            self.buf_state = torch.empty((max_len, *state_dim), dtype=torch.uint8, device=self.device)
        else:
            raise ValueError('state_dim')

    def extend_buffer(self, state, other):  # CPU array to CPU array
        size = len(other)
        next_idx = self.next_idx + size

        if next_idx > self.max_len:
            self.buf_state[self.next_idx:self.max_len] = state[:self.max_len - self.next_idx]
            self.buf_other[self.next_idx:self.max_len] = other[:self.max_len - self.next_idx]
            self.if_full = True

            next_idx = next_idx - self.max_len
            self.buf_state[0:next_idx] = state[-next_idx:]
            self.buf_other[0:next_idx] = other[-next_idx:]
        else:
            self.buf_state[self.next_idx:next_idx] = state
            self.buf_other[self.next_idx:next_idx] = other
        self.next_idx = next_idx

    def sample_batch(self, batch_size) -> tuple:
        indices = rd.randint(self.now_len - 1, size=batch_size)
        r_m_a = self.buf_other[indices]
        return (r_m_a[:, 0:1],
                r_m_a[:, 1:2],
                r_m_a[:, 2:],
                self.buf_state[indices],
                self.buf_state[indices + 1])

    def update_now_len(self):
        self.now_len = self.max_len if self.if_full else self.next_idx