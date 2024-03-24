from fitparse import FitFile

fit_file = FitFile('test.fit')
#test 3-24-2024
with open('power_timestamp.csv', 'w') as file:
    file.write('timestamp,power,altitude,speed\n')
    for record in fit_file.get_messages('record'):
        power = record.get_value('power')
        timestamp = record.get_value('timestamp')
        alt = record.get_value('altitude')
        spd = record.get_value('speed')
        if power is not None and timestamp is not None and alt is not None and spd is not None:
            file.write(f'{timestamp},{power},{alt},{(spd*3600)/1000}\n')