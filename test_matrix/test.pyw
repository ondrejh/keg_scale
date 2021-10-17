import tkinter as tk
from tkinter.colorchooser import askcolor
from subprocess import call, check_output

DOT_SIZE = 20
PRE_SIZE = 60
SEL_HEIGTH = 15

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
        self.parent.title('LED matrix tester')

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
        
        self.r = 0x64
        self.g = 0x64
        self.b = 0x64
        self.col = "#{:02x}{:02x}{:02x}".format(self.r, self.g, self.b)
        self.colText = tk.Label(self.colorFrame, text=self.col)
        self.colText.pack()
        self.colCanvas.create_rectangle(0, 0, PRE_SIZE, PRE_SIZE, fill=self.colText['text'])

        self.fastSelFrame = tk.Frame(self.colorFrame)
        self.fastSelFrame.pack()
        self.redSelDot = tk.Canvas(self.fastSelFrame, width=DOT_SIZE, height=SEL_HEIGTH)
        self.redSelDot.grid(row=0, column=0)
        self.redSelDot.create_rectangle(0, 0, DOT_SIZE, SEL_HEIGTH, fill='#640000')
        self.redSelDot.bind("<Button-1>", lambda e=None, r=64, g=0, b=0: self.set_color(e,r,g,b))
        self.greenSelDot = tk.Canvas(self.fastSelFrame, width=DOT_SIZE, height=SEL_HEIGTH)
        self.greenSelDot.grid(row=0, column=1)
        self.greenSelDot.create_rectangle(0, 0, DOT_SIZE, SEL_HEIGTH, fill='#006400')
        self.greenSelDot.bind("<Button-1>", lambda e=None, r=0, g=64, b=0: self.set_color(e,r,g,b))
        self.blueSelDot = tk.Canvas(self.fastSelFrame, width=DOT_SIZE, height=SEL_HEIGTH)
        self.blueSelDot.grid(row=0, column=2)
        self.blueSelDot.create_rectangle(0, 0, DOT_SIZE, SEL_HEIGTH, fill='#000064')
        self.blueSelDot.bind("<Button-1>", lambda e=None, r=0, g=0, b=64: self.set_color(e,r,g,b))
        self.yellowSelDot = tk.Canvas(self.fastSelFrame, width=DOT_SIZE, height=SEL_HEIGTH)
        self.yellowSelDot.grid(row=1, column=0)
        self.yellowSelDot.create_rectangle(0, 0, DOT_SIZE, SEL_HEIGTH, fill='#646400')
        self.yellowSelDot.bind("<Button-1>", lambda e=None, r=64, g=64, b=0: self.set_color(e,r,g,b))
        self.magentaSelDot = tk.Canvas(self.fastSelFrame, width=DOT_SIZE, height=SEL_HEIGTH)
        self.magentaSelDot.grid(row=1, column=1)
        self.magentaSelDot.create_rectangle(0, 0, DOT_SIZE, SEL_HEIGTH, fill='#640064')
        self.magentaSelDot.bind("<Button-1>", lambda e=None, r=64, g=0, b=64: self.set_color(e,r,g,b))
        self.cyanSelDot = tk.Canvas(self.fastSelFrame, width=DOT_SIZE, height=SEL_HEIGTH)
        self.cyanSelDot.grid(row=1, column=2)
        self.cyanSelDot.create_rectangle(0, 0, DOT_SIZE, SEL_HEIGTH, fill='#006464')
        self.cyanSelDot.bind("<Button-1>", lambda e=None, r=0, g=64, b=64: self.set_color(e,r,g,b))
        self.whiteSelDot = tk.Canvas(self.fastSelFrame, width=DOT_SIZE, height=SEL_HEIGTH)
        self.whiteSelDot.grid(row=2, column=1)
        self.whiteSelDot.create_rectangle(0, 0, DOT_SIZE, SEL_HEIGTH, fill='#646464')
        self.whiteSelDot.bind("<Button-1>", lambda e=None, r=64, g=64, b=64: self.set_color(e,r,g,b))

        self.clearBtn = tk.Button(self.colorFrame, text='Clear', command=self.clear_buffer)
        self.clearBtn.pack()
        
        self.get_buffer()

    def clear_buffer(self):
        for i in range(len(self.dots)):
            self.dots[i].set_color(0, 0, 0)
        self.send_buffer()

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
        url = '{}/buffer'.format(PANEL_IP)
        call(['curl', '-d', buffer, '-X',  'POST', url])
        
    def get_buffer(self):
        url = '{}/buffer'.format(PANEL_IP)
        buffer = check_output(['curl', '-X', 'POST', url])
        if len(buffer) == 6*len(self.dots):
            for i in range(len(self.dots)):
                col = buffer[i*6:i*6+6]
                r = int(col[0:2],16)
                g = int(col[2:4],16)
                b = int(col[4:6],16)
                self.dots[i].set_color(r, g, b)
        
    def set_color(self, event, r, g, b):
        self.r = r
        self.g = g
        self.b = b 
        self.col = '#{:02x}{:02x}{:02x}'.format(r, g, b)
        self.colText = self.col
        self.colCanvas.create_rectangle(0, 0, PRE_SIZE, PRE_SIZE, fill=self.col)

    def color_click(self, event):
        colors = askcolor(title="Color Chooser")
        if colors:
            self.set_color(None, int(colors[1][1:3], 16), int(colors[1][3:5], 16), int(colors[1][5:7], 16))


if __name__ == "__main__":
    root = tk.Tk()
    TestApp(root).pack(side="top", fill="both", expand=True)
    root.mainloop()
