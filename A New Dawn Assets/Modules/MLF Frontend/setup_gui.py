from cx_Freeze import setup, Executable

setup(
        name = "MLF Frontend",
        version = "2.b",
        author = "Devon McAvoy",
        author_email = 'Gen.McAvoyII@GMail.com',
        url = '',
        packages = ['os', 'time', 'tkinter', 'tkinter.constants', 'tkinter._fix', 'os.path', 'ntpath', 'stat', 'genericpath'],
        description = "MLF Frontend created to help users edit MLF Files. Command Line verstion.",
        executables = [Executable("MLF_Frontend_GUI.py")])
