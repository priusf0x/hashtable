import pandas as pd
import statistics
from math import sqrt

df = pd.read_json('data.json')
data = df['data']

mean = statistics.mean(data)
variance = statistics.variance(data)

print(f"({round(float(mean)/10**8, 2)} ± {round((sqrt(variance)/mean * 100), 2)}%) * 10^8 \n")
