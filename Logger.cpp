#include "Logger.h"



void init_logger()
{
    Poco::File check("C:/ngids/Work.log");
    Poco::AutoPtr<Poco::ConsoleChannel> console_channel(new Poco::ConsoleChannel);
    Poco::AutoPtr<Poco::FileChannel> file_channel(new Poco::FileChannel);
    if(!check.exists())
            check.createFile();
    file_channel->setProperty("path", "C:/ngids/Work.log");
    file_channel->setProperty("rotation", "10M");   // Режим ротации файла журнала
    file_channel->setProperty("archive", "timestamp");  // Режим архивирования файлов журнала
   

    Poco::AutoPtr<Poco::PatternFormatter> patternFormatter(
    new Poco::PatternFormatter("[%Y-%m-%d  %H:%M:%s] [%U(%u)] %p: %t"));  //формат вывода времени

    patternFormatter->setProperty("times", "local");  // Форматированное время отображается как местное время

    Poco::AutoPtr<Poco::SplitterChannel> splitter_Channel(new Poco::SplitterChannel);     //вывод в оба потока
    splitter_Channel->addChannel(file_channel);
    splitter_Channel->addChannel(console_channel);

    Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
    new Poco::FormattingChannel(patternFormatter, splitter_Channel));

    Poco::Logger::root().setChannel(formattingChannel);

    log_information("Logger inited!");
}
