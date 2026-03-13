#pragma once

#include <istream>
#include <string>

class BufferedReader {
    std::istream &m_Stream;
    std::string m_Buffer;
    size_t m_BufferPos = 0;

    bool readEnded() const;
public:
    static constexpr int BUFFERED_READER_EOF = -1;

    BufferedReader(std::istream &stream);

    void readLineIfNecessary();

    int read();

    void unread();

    int peek();

    bool eof();

    size_t getBufferPos() const;
};
