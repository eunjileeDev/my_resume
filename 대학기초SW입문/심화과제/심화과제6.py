import turtle
import random


t = turtle.Turtle()
t.speed(0)  


c_list = ['red', 'blue', 'green', 'yellow', 'black', 'skyblue', 'gray']

for _ in range(10):
    x = random.choice(range(-200, 201, 20))
    y = random.choice(range(-200, 201, 20))

    size = random.randint(50, 201)

   
    color = random.choice(c_list)

    t.pencolor(color)

    t.penup()
    t.goto(x, y)
    t.pendown()
    for _ in range(5):
        t.forward(size)
        t.right(144)

