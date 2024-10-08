// clang-format off
#include "icons.h"
#include <stdint.h>

const uint8_t baseScreen[MAX_DATA_SIZE] = {
    //                               ##                                                                                                 //Page 0
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                          #####         #####                       #####          ####          //Page 1
    //                               ##                         #######       #######                     #######        ######        
    //         #            #        ##                        ###   ###     ###   ###                   ###   ###      ###  ###       
    //        ##            ##       ##              ##        ##     ##     ##     ##                   ##     ##      ##    ##       
    //       ###            ###      ##              ##       ##       ##   ##       ##                 ##       ##     ##    ##       
    //      ####################     ##              ##       ##       ##   ##       ##                 ##       ##     ###  ###       
    //     ######################    ##              ##       ##       ##   ##       ##                 ##       ##      ######        
    //    ########################   ##              ##       ##       ##   ##       ##                 ##       ##       ####         
    //    ########################   ##         ############  ##       ##   ##       ##                 ##       ##                     //Page 2
    //     ######################    ##         ############  ##       ##   ##       ##                 ##       ##                    
    //      ####################     ##              ##       ##       ##   ##       ##                 ##       ##                    
    //       ###            ###      ##              ##       ##       ##   ##       ##                 ##       ##                    
    //        ##            ##       ##              ##        ##     ##     ##     ##                   ##     ##                     
    //         #            #        ##              ##        ###   ###     ###   ###         ##        ###   ###                     
    //                               ##              ##         #######       #######          ##         #######                      
    //                               ##                          #####         #####           ##          #####                       
    //                               ##                                                                                                 //Page 3
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    //                               ##                                                                                                
    // ################################################################################################################################ //page 4
    // ################################################################################################################################
    //                               ##                                                                                                
    //                               ##                                                                                                
    //               ##              ##                                                                                                
    //              ####             ##                                                                                                
    //             ######            ##                                                                                                
    //            ########           ##                                                                                                
    //           ##########          ##                          #####         #####                       #####          ####          //Page 5
    //          ############         ##                         #######       #######                     #######        ######        
    //             ######            ##                        ###   ###     ###   ###                   ###   ###      ###  ###       
    //             ######            ##              ##        ##     ##     ##     ##                   ##     ##      ##    ##       
    //             ######            ##              ##       ##       ##   ##       ##                 ##       ##     ##    ##       
    //             ######            ##              ##       ##       ##   ##       ##                 ##       ##     ###  ###       
    //             ######            ##              ##       ##       ##   ##       ##                 ##       ##      ######        
    //             ######            ##              ##       ##       ##   ##       ##                 ##       ##       ####         
    //             ######            ##         ############  ##       ##   ##       ##                 ##       ##                     //Page 6
    //             ######            ##         ############  ##       ##   ##       ##                 ##       ##                    
    //             ######            ##              ##       ##       ##   ##       ##                 ##       ##                    
    //             ######            ##              ##       ##       ##   ##       ##                 ##       ##                    
    //             ######            ##              ##        ##     ##     ##     ##                   ##     ##                     
    //             ######            ##              ##        ###   ###     ###   ###         ##        ###   ###                     
    //          ############         ##              ##         #######       #######          ##         #######                      
    //           ##########          ##                          #####         #####           ##          #####                       
    //            ########           ##                                                                                         ####### //Page 7
    //             ######            ##                                                                                         ### ###
    //              ####             ##                                                                                         ## # ##
    //               ##              ##                                                                                         # ### #
    //                               ##                                                                                         #     #
    //                               ##                                                                                         # ### #
    //                               ##                                                                                         # ### #
    //                               ##                                                                                         #######

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFC, 0x0E, 0x07, 0x03, 0x03, 0x03, 0x07, 0x0E, 0xFC, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0xFC, 0x0E, 0x07, 0x03, 0x03, 0x03, 0x07, 0x0E, 0xFC, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFC, 0x0E, 0x07, 0x03, 0x03, 0x03, 0x07, 0x0E, 0xFC, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x7F, 0x7F, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x0F, 0x3F, 0x70, 0xE0, 0xC0, 0xC0, 0xC0, 0xE0, 0x70, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x3F, 0x70, 0xE0, 0xC0, 0xC0, 0xC0, 0xE0, 0x70, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x3F, 0x70, 0xE0, 0xC0, 0xC0, 0xC0, 0xE0, 0x70, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x83, 0xC3, 0xE3, 0xF3, 0xF3, 0xE3, 0xC3, 0x83, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFC, 0x0E, 0x07, 0x03, 0x03, 0x03, 0x07, 0x0E, 0xFC, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0xFC, 0x0E, 0x07, 0x03, 0x03, 0x03, 0x07, 0x0E, 0xFC, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFC, 0x0E, 0x07, 0x03, 0x03, 0x03, 0x07, 0x0E, 0xFC, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xC0, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xC0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x7F, 0x7F, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x0F, 0x3F, 0x70, 0xE0, 0xC0, 0xC0, 0xC0, 0xE0, 0x70, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x3F, 0x70, 0xE0, 0xC0, 0xC0, 0xC0, 0xE0, 0x70, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x3F, 0x70, 0xE0, 0xC0, 0xC0, 0xC0, 0xE0, 0x70, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F, 0x0F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x87, 0xEB, 0xED, 0xEB, 0x87, 0xFF,
};

const uint8_t relativeReferentialIcon[REFERENCETYPE_NB_BYTES] = {
    // #######
    // #   ###
    // # ## ##
    // # ## ##
    // #  ####
    // # # ###
    // # ## ##
    // #######
    0xFF, 0x81, 0xED, 0xDD, 0xB3, 0xFF, 0xFF,
};

const uint8_t absoluteReferentialIcon[REFERENCETYPE_NB_BYTES] = {
    // #######
    // ### ###
    // ## # ##
    // # ### #
    // #     #
    // # ### #
    // # ### #
    // #######
    0xFF, 0x87, 0xEB, 0xED, 0xEB, 0x87, 0xFF,
};

const uint8_t holdIcon[REFERENCETYPE_NB_BYTES] = {
    // #######
    // # ### #
    // # ### #
    // #     #
    // # ### #
    // # ### #
    // # ### #
    // #######
    0xFF, 0x81, 0xF7, 0xF7, 0xF7, 0x81, 0xFF,
};
