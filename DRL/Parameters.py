battery_parameters={
'capacity':500,# kw
'max_charge':100, # kw
'max_discharge':100, #kw
'efficiency':0.95,
'degradation':0, #euro/kw
'max_soc':0.9,
'min_soc':0.1,
'initial_capacity':0.4}

dg_parameters={
'gen_1':{'power_output_max':500, 'power_output_min':150, 'ramping_up':500, 'ramping_down':500},

'gen_2':{'power_output_max':375, 'power_output_min':100, 'ramping_up':375, 'ramping_down':375},

'gen_3':{'power_output_max':500, 'power_output_min':0, 'ramping_up':500, 'ramping_down':500}}