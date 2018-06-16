import kivy

from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.lang import Builder

import os

Builder.load_string("""
<FileWidget>:
    id: file_widget
    Button
        text: "open"
        on_release: file_widget.open(filechooser.path, filechooser.selection)
    FileChooserListView:
        id: filechooser
        on_selection: file_widget.selected(filechooser.selection)
""")


class FileWidget(BoxLayout):
    def open(self, path, filename):
        # self.file_path = os.path.join(path, filename[0])
        self.file_path = filename[0]
        # print(os.path.join(path, filename[0]))
        print(filename[0])
        self.manager.current =  'main'

    def selected(self, filename):
        print(filename)


class MyApp(App):
    def build(self):
        return MyWidget()


if __name__ == '__main__':
    MyApp().run()
