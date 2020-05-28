import pandas as pd
import numpy as np

data = pd.ExcelFile("/home/adblattmann/Downloads/1_Ergebnistabelle.xlsx")

print(data)

#Get Speiltabelle
data_set = data.parse("Spieltabelle2")

#get first data tow 
data_by_member = data_set.iloc[:88, 15:20]
tmp_list = list()
tmp_list_scaled = list()

for i in range(0,88):
    tmp_data = data_by_member.iloc[i]
    print(tmp_data)
    tmp_cnt = 0
    tmp_sum = 0.0
    for num in tmp_data:
        if not np.isnan(num):
            tmp_cnt += 1
            tmp_sum += (1/num)

    value = tmp_sum/tmp_cnt
    tmp_list.append(value)
    tmp_list_scaled.append((1-value)*3)

#build all together 
result = data_by_member.assign(Result1=tmp_list)
result = data_by_member.assign(Result_Scaled=tmp_list_scaled)

result.to_excel("result.xlsx", sheet_name='Result')
