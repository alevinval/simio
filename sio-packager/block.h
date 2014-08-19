#ifndef BLOCK_H
#define BLOCK_H

class Block
{
    unsigned char name_[65];
    unsigned char sha2_[32];
    unsigned char *buffer_;
    unsigned int size_;
    bool corrupted_;
    bool last_;

    void update_hash();
public:
    Block();
    ~Block();

    unsigned int size() const;
    bool corrupted() const;
    bool integral() const;
    bool last() const;
    const unsigned char *sha2() const;
    const unsigned char *name() const;
    unsigned char * buffer() const;

    void from_buffer(unsigned char *buffer, unsigned int size);
    void from_file(const unsigned char (&sha2)[32], unsigned int size);

    void fetch(unsigned char *buffer);
    void store();

    void set_last(unsigned int size);
    void set_corrupted();
};

#endif //BLOCK_H
