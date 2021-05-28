#ifndef _SOFT_SPI_H
#define _SOFT_SPI_H
#define _SOFTSPI_H_

#define SSPI_MODE0 0
#define SSPI_MODE1 1
#define SSPI_MODE2 2
#define SSPI_MODE3 3

#define MOSI 1
#define MISO 2
#define SCK  3

class SoftSPI {
public:
    SoftSPI(int, int, int, int) {}
    int transfer(char) { return 0; }
    void begin() {}
};

#endif
