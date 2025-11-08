electric = int(input("전기 사용량을 입력하세요. "))
home = input("다자녀 가정인가요? (예 or 아니오)")
print("=====================================")
print(f'사용량 : {electric}kwh')
if electric <400 :
    charge= 1600
    price = 197.9
    
    if electric < 200 :
        charge = 910
        price = 99.3
else  :
    charge= 7300
    price = 280.6
    
ep = charge + price * electric

if home == '예':
    ep = ep - ep*0.2

print(f'기본요금 : {charge}원')
print(f'단가 : {price}원')
print(f'전기 요금(다자녀 할인 적용): {ep:.0f}원')
    
