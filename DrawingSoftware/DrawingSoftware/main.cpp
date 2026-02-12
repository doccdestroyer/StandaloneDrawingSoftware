#include <iostream>
#include <QApplication>
#include <MainWindow.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWindow window =  MainWindow();
    window.show();
    return app.exec();
}



