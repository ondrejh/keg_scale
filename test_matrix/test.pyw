import tkinter as tk


DOT_SIZE = 20

class Dot(tk.Canvas):
    def __init__(self, parent, x, y, click):
        tk.Canvas.__init__(self, parent, width=DOT_SIZE, height=DOT_SIZE)
        self.x = x
        self.y = y
        self.create_rectangle(0, 0, DOT_SIZE, DOT_SIZE, fill='black')
        self.bind("<Button-1>", self.callback)
        self.click = click
        self.r = 0x0
        self.g = 0x0
        self.b = 0x0
    def callback(self, event):
        self.click(self.x, self.y)
    def set_color(self, r, g, b):
        self.r = r
        self.g = g
        self.b = b
        self.create_rectangle(0, 0, DOT_SIZE, DOT_SIZE, fill='#{:02X}{:02X}{:02X}'.format(r, g, b))
    def get_color(self):
        return (self.r, self.g, self.b)

class TestApp(tk.Frame):
    def __init__(self, parent, *args, **kwargs):
        tk.Frame.__init__(self, parent, *args, **kwargs)
        self.parent = parent

        self.matrixFrame = tk.Frame(self.parent)
        self.matrixFrame.pack()
        self.dots = []
        for y in range(8):
            for x in range(32):
                self.dots.append(Dot(self.matrixFrame, x, y, self.click_dot))
                self.dots[-1].grid(row=y, column=x)
    def click_dot(self, x, y):
        n = y * 32 + x
        c = self.dots[n].get_color()
        print(x, y, c)
        self.dots[n].set_color(0,100,0)


if __name__ == "__main__":
    root = tk.Tk()
    TestApp(root).pack(side="top", fill="both", expand=True)
    root.mainloop()
