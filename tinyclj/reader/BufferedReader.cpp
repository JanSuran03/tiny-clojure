#include "BufferedReader.h"
#include <cstring>

bool BufferedReader::readEnded() const {
    return (m_Stream.eof() || m_Stream.bad()) && m_BufferPos >= m_Buffer.size();
}

BufferedReader::BufferedReader(std::istream &stream) : m_Stream(stream) {}

void BufferedReader::readLineIfNecessary() {
    if (m_BufferPos < m_Buffer.size()) {
        return;
    }
    if (m_BufferPos >= m_Buffer.size()) {
        std::string line;
        if (!std::getline(m_Stream, line)) {
            return;
        }
        m_Buffer += line + '\n';
    }
}

int BufferedReader::read() {
    readLineIfNecessary();
    if (readEnded()) {
        return BUFFERED_READER_EOF;
    }
    return m_Buffer[m_BufferPos++];
}

void BufferedReader::unread() {
    if (m_BufferPos > 0) {
        m_BufferPos--;
    }
}

int BufferedReader::peek() {
    readLineIfNecessary();
    if (readEnded()) {
        return BUFFERED_READER_EOF;
    }
    return m_Buffer[m_BufferPos];
}

bool BufferedReader::eof() {
    return m_Stream.eof() && m_BufferPos >= m_Buffer.size();
}

size_t BufferedReader::getBufferPos() const {
    return m_BufferPos;
}
