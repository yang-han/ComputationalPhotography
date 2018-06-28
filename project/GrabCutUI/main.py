from kivy.uix.widget import Widget
from kivy.app import App
# from kivy.properties import ObjectProperty, ListProperty, StringProperty
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.lang import Builder
from kivy.clock import Clock
from kivy.base import runTouchApp
from kivy.uix.gridlayout import GridLayout
from kivy.uix.boxlayout import BoxLayout
# from kivy.uix.image import Image
from kivy.graphics import Color, Line, Rectangle
from kivy.uix.filechooser import FileChooserIconView
# from FileWidget import FileWidget
from enum import Enum, unique
from random import random

Builder.load_string('''
<MyScreenManager>:
    FileScreen
    MainScreen

<FileScreen>:
    name: 'file'
    id: file_widget
    BoxLayout:
        FileChooserListView:
            id: filechooser
            on_selection: file_widget.selected(filechooser.selection)
        Button
            text: "open"
            on_release: file_widget.open(filechooser.path, filechooser.selection)

<MainScreen>:
    name: 'main'
    id: main
''')

Builder.load_string('''
<ButtonTab>
    id: button_tab
    orientation: "vertical"
    Button:
        id: rectangle_mode
        text: "rectangle mode"
        on_release: root.parent.set_rectangle_mode()
    Button:
        id: foreground_points_mode
        text: "foregroud points mode"
        on_release: root.parent.set_foreground_mode()
    Button:
        id: backgroud_points_mode
        text: "backgroud points mode"
        on_release: root.parent.set_backgroud_mode()
    Button:
        id: iter
        text: "iter"
        on_release: root.parent.do_iter()
    Button:
        id: matting
        text: "matting"
        on_release: root.parent.do_matting()
    Button:
        id: reset
        text: "reset"
        on_release: root.parent.do_reset()
    Button:
        id: back_file
        text: "back"
        on_release: root.parent.parent.manager.current = 'file'
''')


class MyScreenManager(ScreenManager):
    # pass
    def __init__(self, **kwargs):
        super(MyScreenManager, self).__init__()
        self.path = None

    def echo(self):
        print('sm: ', self.path)

    def to_main(self):
        print("tomain")
        self.current = 'main'
        print(self.current_screen)
        self.current_screen.set_path(self.path)


class FileScreen(Screen):
    def __init__(self, **kwargs):
        super(FileScreen, self).__init__(**kwargs)
        self.path = '~'

    def open(self, path, filename):
        if len(filename) < 1:
            return
        print(filename[0])
        self.manager.path = filename[0]
        self.manager.to_main()
        self.manager.echo()

    def selected(self, filename):
        print(filename)


class MainScreen(Screen):
    def __init__(self, **kwargs):
        super(MainScreen, self).__init__(**kwargs)
        self.widget = MainWidget()
        self.add_widget(self.widget)

    def set_path(self, path):
        print('setting path.....')
        print(self.widget.canvas_widget.set_origin(path))


class MainWidget(BoxLayout):
    def __init__(self, **kwargs):
        super(MainWidget, self).__init__(**kwargs)
        self.canvas_widget = CanvasWidget()
        self.button_tab = ButtonTab()
        self.add_widget(self.canvas_widget)
        self.add_widget(self.button_tab)

    def do_iter(self):
        print("main itering...")
        self.canvas_widget.do_iter()

    def do_reset(self):
        print("main reseting ...")
        self.canvas_widget.do_reset()
    
    def do_matting(self):
        print("mating...")
        self.canvas_widget.do_matting()

    def set_rectangle_mode(self):
        print("now at RectangleMode")
        self.canvas_widget.now_mode = Mode.RectangleMode

    def set_foreground_mode(self):
        print("now at ForegroundMode")
        self.canvas_widget.now_mode = Mode.ForegroundMode

    def set_backgroud_mode(self):
        print("now at BackgroundMode")
        self.canvas_widget.now_mode = Mode.BackgroundMode


class ButtonTab(BoxLayout):
    pass


@unique
class Mode(Enum):
    NoneMode = 0
    RectangleMode = 1
    ForegroundMode = 2
    BackgroundMode = 3


class CanvasWidget(Widget):
    point_1 = (None, None)
    point_2 = (None, None)
    foreground_points = list()
    background_points = list()
    origin_source = None
    now_mode = Mode.NoneMode
    draw_rect = None
    foreground = None
    background = None
    rect = None

    def __init__(self, **kwargs):
        super(CanvasWidget, self).__init__(**kwargs)
        self.draw_img()
    
    def draw_img(self):        
        with self.canvas:
            self.rect = Rectangle(pos=self.pos, size=self.size)

        self.bind(pos=self.update_rect)
        self.bind(size=self.update_rect)

    def set_origin(self, source):
        self.origin_source = source
        self.update_source(self.origin_source)

    def on_touch_down(self, touch):
        if touch.x > self.size[0] or touch.y > self.size[1]:
            return
        if self.now_mode == Mode.RectangleMode:
            self.point_1 = (touch.x, touch.y)
            print(self.point_1)
        elif self.now_mode == Mode.ForegroundMode:
            with self.canvas:
                Color(1,0,0,0.3, mode="rgba")
                self.foreground = Line(points=(touch.x, touch.y))
        elif self.now_mode == Mode.BackgroundMode:
            with self.canvas:
                Color(0,1,0,0.3,mode="rgba")
                self.background = Line(points=(touch.x, touch.y))

    def on_touch_move(self, touch):
        if touch.x > self.size[0] or touch.y > self.size[1]:
            return
        if self.now_mode == Mode.RectangleMode:
            self.point_2 = (touch.x, touch.y)
            with self.canvas:
                Color(0, 0, 1, 0.3, mode="rgba")
                pos = (min(self.point_1[0], self.point_2[0]), min(self.point_1[1], self.point_2[1]))
                size=(abs(self.point_2[0] - self.point_1[0]),
                      abs(self.point_2[1] - self.point_1[1]))
                print(pos, size)
                print("draw rect")
                if self.draw_rect == None:
                    self.draw_rect=Rectangle(pos = pos, size = size)
                else:
                    self.update_draw_rect(pos, size)
        
        elif self.now_mode == Mode.ForegroundMode:
            with self.canvas:
                Color(1,0,0,0.3, mode="rgba")
                self.foreground.points += (touch.x, touch.y)
        elif self.now_mode == Mode.BackgroundMode:
            with self.canvas:
                Color(0,1,0,0.3,mode="rgba")
                self.background.points += (touch.x, touch.y)


    def on_touch_up(self, touch):
        if touch.x > self.size[0] or touch.y > self.size[1]:
            return
        self.point_2=(touch.x, touch.y)
        print(self.point_2)
        if self.now_mode == Mode.ForegroundMode:
            with self.canvas:
                Color(1,0,0,0.3, mode="rgba")
                self.foreground.points += (touch.x, touch.y)
                self.foreground_points += self.foreground.points
            print(self.foreground_points)
        elif self.now_mode == Mode.BackgroundMode:
            with self.canvas:
                Color(0,1,0,0.3,mode="rgba")
                self.background.points += (touch.x, touch.y)
                self.background_points += self.background.points
            print(self.background_points)

    def update_rect(self, *args):
        self.rect.pos=self.pos
        self.rect.size=self.size

    def update_draw_rect(self, *args):
        self.draw_rect.pos=args[0]
        self.draw_rect.size=args[1]

    def update_source(self, source):
        self.rect.source=source

    def do_reset(self):
        self.point_1=(None, None)
        self.point_2=(None, None)
        self.foreground_points=list()
        self.background_points=list()
        self.now_mode=Mode.NoneMode
        self.draw_rect=None
        self.update_source(self.origin_source)
    
    def do_matting(self):
        pass

    def do_iter(self):
        if self.point_1 == (None, None):
            return
        print('---')
        print(self.point_1)
        print(self.size)
        point_ratio_1=(self.point_1[0] / self.size[0],
                         self.point_1[1] / self.size[1])
        point_ratio_2=(self.point_2[0] / self.size[0],
                         self.point_2[1] / self.size[1])
        print(point_ratio_1, point_ratio_2)
        
        # grabcut(...)
        # self.update_source(...)

        # clear and display new image
        self.canvas.clear()
        self.point_1=(None, None)
        self.point_2=(None, None)
        self.foreground_points=list()
        self.background_points=list()
        self.now_mode=Mode.NoneMode
        self.draw_rect=None
        self.draw_img()
        self.update_source(self.origin_source) # new img


class ImageWidget(Widget):
    def set_source(self, source):
        print("setting source")
        self.source=source


Builder.load_string("""
<ImageWidget>:
    canvas:
        Rectangle:
            id: img
            pos: self.pos
            size: self.size
            source: 'texture_example_image.png'
""")


class MainApp(App):
    def build(self):
        return MyScreenManager()


MainApp().run()
