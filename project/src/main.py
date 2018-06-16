from kivy.uix.widget import Widget
from kivy.app import App
from kivy.properties import ObjectProperty, ListProperty, StringProperty
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.lang import Builder
from kivy.clock import Clock
from kivy.base import runTouchApp
from kivy.uix.gridlayout import GridLayout
# from kivy.uix.image import Image
from kivy.uix.filechooser import FileChooserIconView
from FileWidget import FileWidget

Builder.load_string('''
<MyScreenManager>:
    FileScreen
    MainScreen

<FileScreen>:
    name: 'file'
    id: file_widget
    BoxLayout:
        Button
            text: "open"
            on_release: file_widget.open(filechooser.path, filechooser.selection)
        FileChooserListView:
            id: filechooser
            on_selection: file_widget.selected(filechooser.selection)

<MainScreen>:
    name: 'main'
    id: main
    Button:
        text: "Go back home"
        on_release: root.manager.to_main()
''')


class MyScreenManager(ScreenManager):
    # pass
    def __init__(self, **kwargs):
        super(MyScreenManager, self).__init__()
        self.path = None

    def echo(self):
        print('sm: ', self.path)
    
    def to_main(self):
        # self.main.set_path(self.path)
        print("tomain")
        self.current = 'main'
        print(self.current_screen)
        self.current_screen.set_path(self.path)

class FileScreen(Screen):
    def __init__(self, **kwargs):
        super(FileScreen, self).__init__(**kwargs)
        # self.filechooser = FileWidget()
        # self.add_widget(self.filechooser)
        self.path = '~'

    def open(self, path, filename):
        # self.file_path = filename[0]
        print(filename[0])
        self.manager.path = filename[0]
        self.manager.to_main()
        self.manager.echo()

    def selected(self, filename):
        print(filename)


class MainScreen(Screen):
    def __init__(self, **kwargs):
        super(MainScreen, self).__init__(**kwargs)
        self.img = ImageWidget()
        self.add_widget(self.img)
    
    def set_path(self, path):
        print('setting path.....')
        print(self.ids)
        self.img.source = path
        


# class MyWidget(GridLayout):
#     def __init__(self, **kwargs):
#         super(MyWidget, self).__init__(**kwargs)
#         self.id = "my_widget"
#         self.cols = 2
#         self.image = ImageWidget()
#         self.add_widget(self.image)
#         self.add_widget(self.filechooser)

#     def fuck():
#         print("fuck")


class ImageWidget(Widget):
    pass


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

# root = MyScreenManager()
# runTouchApp(root)
