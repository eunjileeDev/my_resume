AB = 1
AC = 1
BC = 1

while (True):
    hour1 = ( 50 * AB ) / 60 + 9
    minute1 = ( 50 * AB ) % 60
    if ( hour1 <= 22 ):
        print("A, B 열차 : %d시 %d분" %(hour1, minute1))
    else:
        break
    AB += 1

while (True):
    hour2 = ( 30 * AC ) / 60 + 9
    minute2 = ( 30 * AC ) % 60
    if ( hour2 <= 22 ):
        print("A, C 열차 : %d시 %d분" %(hour2, minute2))
    else:
        break
    AC += 1

while (True):
    hour3 = ( 150 * BC ) / 60 + 9
    minute3 = ( 150 * BC ) % 60
    if ( hour3 <= 22 ):
        print("B, C 열차 : %d시 %d분" %(hour3, minute3))
    else:
        break
    BC += 1

     
