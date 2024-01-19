import json 
from tabulate import tabulate
import pandas as pd
f=open('test.json')

data = json.load(f)

length =len (data)

answers = []

for i in range(length):
	try: 
		x=data[i]['_source']['layers']['dns']['Answers'].values()
		#print(data[i]['_source']['layers']['dns']['Answers'])
		for i in x:
			try:
				if(i['dns.a'] and i['dns.resp.name']):
					answer = {}
					print(i['dns.resp.name'])
					answer["resp_name"] = i['dns.resp.name']
					print(i['dns.a'])
					answer["a"] = i['dns.a']
					#print('\n');

					answers.append(answer)
			except:
				continue 
		
	except: 
		continue


df = pd.DataFrame(answers)

with open("ans.csv", "w") as f:
	# save in csv format
	f.write(df.to_csv(sep="\t", index=False))
	


f.close()