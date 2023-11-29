# ------------------------------------------------------------------------
# Energy management environment for reinforcement learning agents
# Modified by Talha Rehman
# Original by Hou Shengren, TU Delft, h.shengren@tudelft.nl
import random
import numpy as np
import pandas as pd
import gym
from gym import spaces

from Parameters import battery_parameters, dg_parameters


class Constant:
    MONTHS_LEN = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    MAX_STEP_HOURS = 24 * 30


class DataManager():
    def __init__(self) -> None:
        self.PV_Generation = []
        self.Prices = []
        self.Electricity_Consumption = []

    def add_pv_element(self, element): self.PV_Generation.append(element)

    def add_price_element(self, element): self.Prices.append(element)

    def add_electricity_element(self, element): self.Electricity_Consumption.append(element)

    # get current time data based on given month day, and day_time
    def get_pv_data(self, month, day, day_time): return self.PV_Generation[
        (sum(Constant.MONTHS_LEN[:month - 1]) + day - 1) * 24 + day_time]

    def get_price_data(self, month, day, day_time): return self.Prices[
        (sum(Constant.MONTHS_LEN[:month - 1]) + day - 1) * 24 + day_time]

    def get_electricity_cons_data(self, month, day, day_time): return self.Electricity_Consumption[
        (sum(Constant.MONTHS_LEN[:month - 1]) + day - 1) * 24 + day_time]

    # get series data for one episode
    def get_series_pv_data(self, month, day): return self.PV_Generation[
                                                     (sum(Constant.MONTHS_LEN[:month - 1]) + day - 1) * 24:(
                                                                                                                       sum(Constant.MONTHS_LEN[
                                                                                                                           :month - 1]) + day - 1) * 24 + 24]

    def get_series_price_data(self, month, day): return self.Prices[
                                                        (sum(Constant.MONTHS_LEN[:month - 1]) + day - 1) * 24:(
                                                                                                                          sum(Constant.MONTHS_LEN[
                                                                                                                              :month - 1]) + day - 1) * 24 + 24]

    def get_series_electricity_cons_data(self, month, day): return self.Electricity_Consumption[(
                                                                                                            sum(Constant.MONTHS_LEN[
                                                                                                                :month - 1]) + day - 1) * 24:(
                                                                                                                                                         sum(Constant.MONTHS_LEN[
                                                                                                                                                             :month - 1]) + day - 1) * 24 + 24]


class DG():
    '''simulate a simple diesel generator here'''

    def __init__(self, parameters):
        self.name = parameters.keys()
        self.power_output_max = parameters['power_output_max']
        self.power_output_min = parameters['power_output_min']
        self.ramping_up = parameters['ramping_up']
        self.ramping_down = parameters['ramping_down']
        self.last_step_output = None

    def step(self, action_gen):
        output_change = action_gen * self.ramping_up  # constrain the output_change with ramping up boundary
        output = self.current_output + output_change
        if output > 0:
            output = max(self.power_output_min, min(self.power_output_max, output))  # meet the constraint
        else:
            output = 0
        self.current_output = output

    def _get_cost(self, output):
        cost = 150
        return cost

    def reset(self):
        self.current_output = 0


class Battery():
    '''simulate a simple battery here'''

    def __init__(self, parameters):
        self.capacity = parameters['capacity']
        self.max_soc = parameters['max_soc']
        self.initial_capacity = parameters['initial_capacity']
        self.min_soc = parameters['min_soc']
        self.degradation = parameters['degradation']
        self.max_charge = parameters['max_charge']
        self.max_discharge = parameters['max_discharge']
        self.efficiency = parameters['efficiency']

    def step(self, action_battery):
        energy = action_battery * self.max_charge
        updated_capacity = max(self.min_soc,
                               min(self.max_soc, (self.current_capacity * self.capacity + energy) / self.capacity))
        self.energy_change = (
                                         updated_capacity - self.current_capacity) * self.capacity  # if charge, positive, if discharge, negative
        self.current_capacity = updated_capacity  # update capacity to current codition

    def _get_cost(self, energy):  # calculate the cost depends on the energy change
        cost = energy ** 2 * self.degradation
        return cost

    def SOC(self):
        return self.current_capacity

    def reset(self):
        self.current_capacity = np.random.uniform(0.1, 0.9)


class Grid():
    def __init__(self):

        self.on = True
        if self.on:
            self.exchange_ability = 500
        else:
            self.exchange_ability = 0

    def _get_cost(self, current_price, energy_exchange):
        return current_price * energy_exchange




class ESSEnv(gym.Env):
    def __init__(self, **kwargs):
        super(ESSEnv, self).__init__()
        # parameters
        self.data_manager = DataManager()
        self._load_year_data()
        self.episode_length = kwargs.get('episode_length', 24)
        self.month = None
        self.day = None
        self.TRAIN = True
        self.current_time = None
        self.battery_parameters = kwargs.get('battery_parameters', battery_parameters)
        self.dg_parameters = kwargs.get('dg_parameters', dg_parameters)
        self.penalty_coefficient = 50  # control soft penalty constrain
        self.sell_coefficient = 0.5  # control sell benefits

        self.grid = Grid()
        self.battery = Battery(self.battery_parameters)
        self.dg1 = DG(self.dg_parameters['gen_1'])
        self.dg2 = DG(self.dg_parameters['gen_2'])
        self.dg3 = DG(self.dg_parameters['gen_3'])

        self.action_space = spaces.Box(low=-1, high=1, shape=(4,), dtype=np.float32)

        self.state_space = spaces.Box(low=0, high=1, shape=(7,), dtype=np.float32)

    def reset(self, ):
        self.month = np.random.randint(1, 13)  # here we choose 12 month
        if self.TRAIN:
            self.day = np.random.randint(1, 20)
        else:
            print(self.month)
            self.day = np.random.randint(20, Constant.MONTHS_LEN[self.month-1])
        self.current_time = 0
        self.battery.reset()
        self.dg1.reset()
        self.dg2.reset()
        self.dg3.reset()
        return self._build_state()

    def _build_state(self):
        soc = self.battery.SOC()
        dg1_output = self.dg1.current_output
        dg2_output = self.dg2.current_output
        dg3_output = self.dg3.current_output
        time_step = self.current_time
        electricity_demand = self.data_manager.get_electricity_cons_data(self.month, self.day, self.current_time)
        pv_generation = self.data_manager.get_pv_data(self.month, self.day, self.current_time)
        price = self.data_manager.get_price_data(self.month, self.day, self.current_time)
        net_load = electricity_demand - pv_generation
        obs = np.concatenate((np.float32(time_step), np.float32(price), np.float32(soc), np.float32(net_load),
                              np.float32(dg1_output), np.float32(dg2_output), np.float32(dg3_output)), axis=None)
        return obs

    def step(self, action):  # state transition here current_obs--take_action--get reward-- get_finish--next_obs
        ## here we want to put take action into each components
        current_obs = self._build_state()
        self.battery.step(action[0])  # here execute the state-transition part, battery.current_capacity also changed
        self.dg1.step(action[1])
        self.dg2.step(action[2])
        self.dg3.step(action[3])
        current_output = np.array((self.dg1.current_output, self.dg2.current_output, self.dg3.current_output,
                                   -self.battery.energy_change))  # truely corresonding to the result
        self.current_output = current_output
        actual_production = sum(current_output)
        netload = current_obs[3]
        price = current_obs[1]

        unbalance = actual_production - netload

        reward = 0
        excess_penalty = 0
        deficient_penalty = 0
        sell_benefit = 0
        buy_cost = 0
        self.excess = 0
        self.shedding = 0
        if unbalance >= 0:  # it is now in excess condition
            if unbalance <= self.grid.exchange_ability:
                sell_benefit = self.grid._get_cost(price,
                                                   unbalance) * self.sell_coefficient  # sell money to grid is little [0.029,0.1]
            else:
                sell_benefit = self.grid._get_cost(price, self.grid.exchange_ability) * self.sell_coefficient
                # real unbalance that even grid could not meet
                self.excess = unbalance - self.grid.exchange_ability
                excess_penalty = self.excess * self.penalty_coefficient
        else:  # unbalance <0, its load shedding model, in this case, deficient penalty is used
            if abs(unbalance) <= self.grid.exchange_ability:
                buy_cost = self.grid._get_cost(price, abs(unbalance))
            else:
                buy_cost = self.grid._get_cost(price, self.grid.exchange_ability)
                self.shedding = abs(unbalance) - self.grid.exchange_ability
                deficient_penalty = self.shedding * self.penalty_coefficient
        battery_cost = self.battery._get_cost(self.battery.energy_change)  # we set it as 0 this time
        dg1_cost = self.dg1._get_cost(self.dg1.current_output)
        dg2_cost = self.dg2._get_cost(self.dg2.current_output)
        dg3_cost = self.dg3._get_cost(self.dg3.current_output)

        reward -= (battery_cost + dg1_cost + dg2_cost + dg3_cost + excess_penalty +
                   deficient_penalty - sell_benefit + buy_cost) / 1e3
        self.operation_cost = battery_cost + dg1_cost + dg2_cost + dg3_cost + buy_cost - sell_benefit + excess_penalty + deficient_penalty
        self.unbalance = unbalance
        self.real_unbalance = self.shedding + self.excess
        final_step_outputs = [self.dg1.current_output, self.dg2.current_output, self.dg3.current_output,
                              self.battery.current_capacity]
        self.current_time += 1
        finish = (self.current_time == self.episode_length)
        if finish:
            self.final_step_outputs = final_step_outputs
            self.current_time = 0
            # self.day+=1
            # if self.day>Constant.MONTHS_LEN[self.month-1]:
            #     self.day=1
            #     self.month+=1
            # if self.month>12:
            #     self.month=1
            #     self.day=1
            next_obs = self.reset()

        else:
            next_obs = self._build_state()
        return current_obs, next_obs, float(reward), finish

    def render(self, current_obs, next_obs, reward, finish):
        print('day={},hour={:2d}, state={}, next_state={}, reward={:.4f}, terminal={}\n'.format(self.day,
                                                                                                self.current_time,
                                                                                                current_obs, next_obs,
                                                                                                reward, finish))

    def _load_year_data(self):
        pv_df = pd.read_csv('./Data/PVdata.csv')
        # hourly price data for a year
        price_df = pd.read_csv('Data/Price.csv')
        # Hourly electricity consumption data for a year
        electricity_df = pd.read_csv('./Data/Load.csv')

        pv_data = pv_df['Power'].to_numpy(dtype=float)
        price = price_df['Price'].to_numpy(dtype=float)
        electricity = electricity_df['Load'].to_numpy(dtype=float)

        for element in pv_data:
            self.data_manager.add_pv_element(element)
        for element in price:
            element /= 10
            if element <= 0.5:
                element = 0.5
            self.data_manager.add_price_element(element)
        for element in electricity:
            self.data_manager.add_electricity_element(element)


if __name__ == '__main__':
    env = ESSEnv()
    env.TRAIN = False
    rewards = []

    current_obs = env.reset()
    tem_action = [0.1, 0.1, 0.1, 0.1]
    for _ in range(144):
        print(f'current month is {env.month}, current day is {env.day}, current time is {env.current_time}')
        current_obs, next_obs, reward, finish = env.step(tem_action)
        env.render(current_obs, next_obs, reward, finish)
        current_obs = next_obs
        rewards.append(reward)