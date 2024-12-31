#ifndef WINDOWDATA_H
#define WINDOWDATA_H

#define WINDOWDATA_MAX_CHAR_LEN 72 //Need to find this

class WindowData
{
    private:
        char socketMessage[WINDOWDATA_MAX_CHAR_LEN];
    public:
        WindowData();
        void setMessage(const char message[]);
        const char* getMessage() const;
};

#endif