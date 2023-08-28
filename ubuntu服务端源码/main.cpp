#include<QCoreApplication>
#include"Server.h"
int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	Server server(nullptr);
    Packet<Server> p(nullptr);
	return app.exec();
}
