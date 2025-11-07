n = int(input("n의 값은?"))

total = 0

for i in range(2, n+1, 2):
    total = total + i
print(f'2부터 {n}까지의 짝수의 합: {total}')
