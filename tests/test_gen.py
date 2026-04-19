import random as rd

with open('tests/en_dictionary.txt', 'r') as file:
    words_dict = file.readlines()
    words_dict = [s.strip("\n") for s in words_dict]

with open('tests/extracted_words.txt', 'r') as file:
    words_extracted = file.readlines()
    words_extracted = [s.strip("\n") for s in words_extracted]

words = words_dict + words_extracted 

test_number = 1000000
with open('tests/test.txt', 'w+') as file: 
    for i in range(test_number):
        file.write(rd.choice(words) + "\n") 

