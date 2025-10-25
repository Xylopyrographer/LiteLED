//
/*
    LiteLED RMT Encoder Callback Implementation
*/

#include "ll_encoder.h"

size_t led_encoder_cb( const void* data, size_t data_size,
                       size_t symbols_written, size_t symbols_free,
                       rmt_symbol_word_t *symbols, bool *done, void *arg ) {

    // We need a minimum of 8 symbol spaces to encode a byte. We only
    // need one to encode a reset, but it's simpler to simply demand that
    // there are 8 symbol spaces free to write anything.
    if ( symbols_free < 8 ) {
        return 0;
    }

    led_strip_t *strip = ( led_strip_t* )arg; // Cast arg to led_strip_t*
    uint8_t *data_bytes = ( uint8_t* )data;
    uint8_t photons = strip->brightness;      // Get the brightness value

    // Retrieve the current position from enc_pos
    size_t data_pos = strip->stripCfg.enc_pos;

    if ( data_pos < data_size ) {
        // Encode a data byte with brightness scaling
        uint8_t currentByte = scale8_video( data_bytes[ data_pos ], photons );

        // Convert the byte to RMT symbols (8 bits = 8 symbols)
        size_t symbol_pos = 0;
        for ( int bitmask = 0x80; bitmask != 0; bitmask >>= 1 ) {
            symbols[ symbol_pos++ ] = ( currentByte & bitmask ) ?
                                      led_params[ strip->type ].led_1 :
                                      led_params[ strip->type ].led_0;
        }

        // Update the current position in the buffer
        strip->stripCfg.enc_pos++;

        // We should have written 8 symbols
        return symbol_pos;
    }
    else {
        // All bytes have been encoded.
        // Encode the reset symbol, and we're done.
        symbols[ 0 ] = led_params[ strip->type ].led_reset;
        strip->stripCfg.enc_pos = 0;    // Reset the position in the buffer
        *done = 1;                      // Indicate end of the transaction
        return 1;                       // We only wrote one symbol
    }
}

//  --- EOF --- //
