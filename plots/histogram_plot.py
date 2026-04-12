import pandas as pd
import statistics
import matplotlib.pyplot as plt
from math import sqrt

df = pd.read_json('HashRor.json')
data = df['data']

index = [i for i in range(len(data))]
plt.bar(index,data)
plt.title("Bar for HASHROL hash function")
plt.xlabel("Buckets")
plt.ylabel("Words")
plt.savefig("meow.png")

mean = statistics.mean(data)
variance = statistics.variance(data)

# print(f"({round(float(mean)/10**6, 2)} ± {round((sqrt(variance)/mean * 100), 2)}%) * 10^6 \n")
