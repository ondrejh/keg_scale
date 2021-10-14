import tkinter as tk
from tkinter.colorchooser import askcolor
from subprocess import call

DOT_SIZE = 20
PRE_SIZE = 60

PANEL_IP = '192.168.88.221'


class Dot(tk.Canvas):
    def __init__(self, parent, x, y, click, click2):
        tk.Canvas.__init__(self, parent, width=DOT_SIZE, height=DOT_SIZE)
        self.x = x
        self.y = y
        self.create_rectangle(0, 0, DOT_SIZE, DOT_SIZE, fill='black')
        self.bind("<Button-1>", self.callback)
        self.bind("<Button-2>", self.callback2)
        self.bind("<Button-3>", self.callback2)
        self.click = click
        self.click2 = click2
        self.r = 0x0
        self.g = 0x0
        self.b = 0x0

    def callback(self, event):
        self.click(self.x, self.y)
    def callback2(self, event):
        self.click2(self.x, self.y)

    def set_color(self, r, g, b):
        self.r = r
        self.g = g
        self.b = b
        self.create_rectangle(0, 0, DOT_SIZE, DOT_SIZE, fill='#{:02X}{:02X}{:02X}'.format(r, g, b))

    def get_color(self):
        return self.r, self.g, self.b


class TestApp(tk.Frame):
    def __init__(self, parent, *args, **kwargs):
        tk.Frame.__init__(self, parent, *args, **kwargs)
        self.parent = parent

        self.matrixFrame = tk.LabelFrame(self.parent, text='Dot Matrix')
        self.matrixFrame.pack(side=tk.LEFT, padx=5, pady=5, expand=1, fill=tk.BOTH)
        self.dots = []
        for y in range(8):
            for x in range(32):
                self.dots.append(Dot(self.matrixFrame, x, y, self.click_dot, self.click2_dot))
                self.dots[-1].grid(row=y, column=x)

        self.colorFrame = tk.LabelFrame(self.parent, text='Color')
        self.colorFrame.pack(side=tk.LEFT, padx=5, pady=5, expand=1, fill=tk.BOTH)
        self.colCanvas = tk.Canvas(self.colorFrame, width=PRE_SIZE, height=PRE_SIZE)
        self.colCanvas.pack()
        self.colCanvas.bind("<Button-1>", self.color_click)
        
        self.r = 0xFF
        self.g = 0xFF
        self.b = 0xFF
        self.col = "#{:02x}{:02x}{:02x}".format(self.r, self.g, self.b)
        self.colText = tk.Label(self.colorFrame, text=self.col)
        self.colText.pack()
        self.colCanvas.create_rectangle(0, 0, PRE_SIZE, PRE_SIZE, fill=self.colText['text'])

    def click_dot(self, x, y):
        n = y * 32 + x
        self.dots[n].set_color(self.r, self.g, self.b)
        self.send_buffer()

    def click2_dot(self, x, y):
        n = y * 32 + x 
        self.dots[n].set_color(0, 0, 0)
        self.send_buffer()
        
    def send_buffer(self):
        buffer = "buffer="
        for i in range(len(self.dots)):
            c = self.dots[i].get_color()
            buffer += "{:02X}{:02X}{:02X}".format(c[0], c[1], c[2])
        print(buffer)
        url = '{}/buffer'.format(PANEL_IP)
        call(['curl', '-d', buffer, '-X',  'POST', url])
        

    def color_click(self, event):
        colors = askcolor(title="Tkinter Color Chooser")
        if colors:
            self.r = int(colors[1][1:3], 16)
            self.g = int(colors[1][3:5], 16)
            self.b = int(colors[1][5:7], 16)
            self.col = colors[1]
            self.colText['text'] = self.col
            self.colCanvas.create_rectangle(0, 0, PRE_SIZE, PRE_SIZE, fill=self.colText['text'])


if __name__ == "__main__":
    root = tk.Tk()
    TestApp(root).pack(side="top", fill="both", expand=True)
    root.mainloop()
