#include "hw_config.h"
#include "sd_card.h"
#include "spi.h"

// Define the SPI bus configuration (SPI0)
static spi_t spi_device = {
    .hw_inst = spi0,
    .miso_gpio = 0,
    .sck_gpio = 2,
    .mosi_gpio = 3,
    .baud_rate = 12500000
};

// Define the SD card configuration using that SPI bus
static sd_card_t sd_cards[] = {
    {
        .pcName = "0:",
        .spi = &spi_device,
        .ss_gpio = 1
    }
};

size_t sd_get_num() { 
    return count_of(sd_cards); 
}

sd_card_t *sd_get_by_num(size_t num) {
    if (num < count_of(sd_cards)) {
        return &sd_cards[num];
    }
    return NULL;
}

size_t spi_get_num() {
    return 1;
}

spi_t *spi_get_by_num(size_t num) {
    if (num == 0) {
        return &spi_device;
    }
    return NULL;
}