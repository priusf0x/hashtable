import pandas as pd
import statistics
import matplotlib.pyplot as plt
from math import sqrt

df = pd.read_json('plots/HashRol.json')
data = df['data']

# index = [i for i in range(len(data))]
# plt.bar(index,data)
# plt.title("Bar for ELEMSUM hash function")
# plt.xlabel("Buckets")
# plt.ylabel("Words")
# plt.savefig("plots/ElemSum500.png")

mean = statistics.mean(data)
variance = statistics.variance(data)

print(f"varianve={variance}\n")
