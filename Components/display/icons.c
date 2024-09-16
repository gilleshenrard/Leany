/**
 * @file icons.c
 * @author Gilles Henrard
 * @brief Declare icons bitmaps where each bit represent either a background or a foreground pixel
 * @date 13/09/2024
 * 
 * @copyright Copyright (c) 2024
 * 
 *  * @details Generated with the application The Dot Factory v0.1.4
 * 
 * Software Github page : https://github.com/pavius/the-dot-factory
 *
 * 1. generate bitmaps
 *      - characters : 0123456789+-.°
 *      - Verdana 48pts
 *      - Height and width fixed
 *      - Line wrap at column
 *      - Do not generate descriptors
 *      - Comments : Bitmap visualiser and char descriptor
 *      - Byte : RowMajor, MSB first, Format Hex, leading 0x
 *      - Variable format : Bitmaps "const uint64_t {0}Bitmaps"
 * 2. copy bitmaps only
 * 3. remove all ", 0x" to merge row bytes together
 * 4. enclose each bitmap with carrets and a comma
 * 5. transform the array in a two-dimensions array
 * 6. move numbers at first places to take advantage of indexes in the array
 * 7. assign each bitmap array to an enum entry
 */

#include "icons.h"
#include <stdint.h>

typedef uint64_t line_t;

extern const line_t verdana_48ptBitmaps[NB_CHARACTERS][VERDANA_NB_ROWS];

void uncompressIconLine(registerValue_t* buffer, verdanaCharacter_e character, uint8_t line) {
    const registerValue_t MSB_DOWNSHIFT = 8U;
    const registerValue_t LSB_MASK      = 0xFFU;

    line_t pixelMask = ((line_t)1U << ((line_t)VERDANA_NB_COLUMNS - 1U));

    while(pixelMask > 0) {
        pixel_t colour = ((verdana_48ptBitmaps[character][line] & pixelMask) ? BRIGHT_GRAY : DARK_CHARCOAL);
        *(buffer++)    = (registerValue_t)(colour >> MSB_DOWNSHIFT);
        *(buffer++)    = (registerValue_t)(colour & LSB_MASK);

        pixelMask >>= 1U;
    }
}

const line_t verdana_48ptBitmaps[NB_CHARACTERS][VERDANA_NB_ROWS] =
    {
        // clang-format off
    [VERDANA_0] = {
        /* @735 '0' (39 pixels wide) */
        0x0003FE0000,  //               #########
        0x001FFFC000,  //            ###############
        0x007FFFF000,  //          ###################
        0x00FFFFF800,  //         #####################
        0x01FFFFFC00,  //        #######################
        0x03FE03FE00,  //       #########       #########
        0x03F800FE00,  //       #######           #######
        0x07F0007F00,  //      #######             #######
        0x07E0003F00,  //      ######               ######
        0x0FC0001F80,  //     ######                 ######
        0x0FC0001F80,  //     ######                 ######
        0x0F80000F80,  //     #####                   #####
        0x1F80000FC0,  //    ######                   ######
        0x1F80000FC0,  //    ######                   ######
        0x1F80000FC0,  //    ######                   ######
        0x1F800007C0,  //    ######                    #####
        0x3F000007C0,  //   ######                     #####
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x1F00000FC0,  //    #####                    ######
        0x1F80000FC0,  //    ######                   ######
        0x1F80000FC0,  //    ######                   ######
        0x1F80000FC0,  //    ######                   ######
        0x0F80000F80,  //     #####                   #####
        0x0FC0001F80,  //     ######                 ######
        0x0FC0001F80,  //     ######                 ######
        0x07E0003F00,  //      ######               ######
        0x07F0007F00,  //      #######             #######
        0x03F800FE00,  //       #######           #######
        0x03FE03FE00,  //       #########       #########
        0x01FFFFFC00,  //        #######################
        0x00FFFFF800,  //         #####################
        0x007FFFF000,  //          ###################
        0x001FFFC000,  //            ###############
        0x0003FE0000,  //               #########
    },

    [VERDANA_1] = {
        /* @980 '1' (39 pixels wide) */
        0x0000000000,  //
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x0000FC0000,  //                 ######
        0x0003FC0000,  //               ########
        0x000FFC0000,  //             ##########
        0x03FFFC0000,  //       ################
        0x03FFFC0000,  //       ################
        0x03FFFC0000,  //       ################
        0x03FFFC0000,  //       ################
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x0000FC0000,  //                 ######
        0x03FFFFFF00,  //       ##########################
        0x03FFFFFF00,  //       ##########################
        0x03FFFFFF00,  //       ##########################
        0x03FFFFFF00,  //       ##########################
        0x03FFFFFF00,  //       ##########################
        0x0000000000,  //
    },

    [VERDANA_2] = {
        /* @1225 '2' (39 pixels wide) */
        0x000FFC0000,  //             ##########
        0x00FFFFC000,  //         ##################
        0x07FFFFE000,  //      ######################
        0x0FFFFFF800,  //     #########################
        0x0FFFFFFC00,  //     ##########################
        0x0FF807FE00,  //     #########        ##########
        0x0F8000FE00,  //     #####               #######
        0x0E00007F00,  //     ###                  #######
        0x0800003F00,  //     #                     ######
        0x0000003F80,  //                           #######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000003F00,  //                           ######
        0x0000003F00,  //                           ######
        0x0000007F00,  //                          #######
        0x0000007E00,  //                          ######
        0x000000FE00,  //                         #######
        0x000001FC00,  //                        #######
        0x000001FC00,  //                        #######
        0x000003F800,  //                       #######
        0x000007F000,  //                      #######
        0x00000FE000,  //                     #######
        0x00001FC000,  //                    #######
        0x00003FC000,  //                   ########
        0x00007F8000,  //                  ########
        0x0000FF0000,  //                 ########
        0x0001FE0000,  //                ########
        0x0003F80000,  //               #######
        0x0007F00000,  //              #######
        0x000FE00000,  //             #######
        0x003FC00000,  //           ########
        0x007F800000,  //          ########
        0x00FF000000,  //         ########
        0x01FE000000,  //        ########
        0x03FC000000,  //       ########
        0x07F8000000,  //      ########
        0x1FE0000000,  //    ########
        0x1FC0000000,  //    #######
        0x1FFFFFFFE0,  //    ################################
        0x1FFFFFFFE0,  //    ################################
        0x1FFFFFFFE0,  //    ################################
        0x1FFFFFFFE0,  //    ################################
        0x1FFFFFFFE0,  //    ################################
        0x0000000000,  //
    },

    [VERDANA_3] = {
        /* @1470 '3' (39 pixels wide) */
        0x000FFE0000,  //             ###########
        0x00FFFFC000,  //         ##################
        0x07FFFFF000,  //      #######################
        0x0FFFFFF800,  //     #########################
        0x0FFFFFFE00,  //     ###########################
        0x0FF803FE00,  //     #########         #########
        0x0FC000FF00,  //     ######              ########
        0x0E00007F00,  //     ###                  #######
        0x0800003F80,  //     #                     #######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000003F00,  //                           ######
        0x0000003F00,  //                           ######
        0x0000007E00,  //                          ######
        0x000001FC00,  //                        #######
        0x000007F800,  //                      ########
        0x0003FFE000,  //               #############
        0x0003FF8000,  //               ###########
        0x0003FF8000,  //               ###########
        0x0003FFF000,  //               ##############
        0x0003FFF800,  //               ###############
        0x000003FE00,  //                       #########
        0x000000FF00,  //                         ########
        0x0000003F00,  //                           ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x1000003F80,  //    #                      #######
        0x1C00007F00,  //    ###                   #######
        0x1F8001FE00,  //    ######              ########
        0x1FF007FE00,  //    #########         ##########
        0x1FFFFFFC00,  //    ###########################
        0x1FFFFFF800,  //    ##########################
        0x0FFFFFE000,  //     #######################
        0x01FFFF8000,  //        ##################
        0x001FFC0000,  //            ###########
    },

    [VERDANA_4] = {
        /* @1715 '4' (39 pixels wide) */
        0x0000000000,  //
        0x000003F000,  //                       ######
        0x000007F000,  //                      #######
        0x00000FF000,  //                     ########
        0x00001FF000,  //                    #########
        0x00003FF000,  //                   ##########
        0x00003FF000,  //                   ##########
        0x00007FF000,  //                  ###########
        0x0000FFF000,  //                 ############
        0x0001FFF000,  //                #############
        0x0003FBF000,  //               ####### ######
        0x0007F3F000,  //              #######  ######
        0x0007E3F000,  //              ######   ######
        0x000FC3F000,  //             ######    ######
        0x001F83F000,  //            ######     ######
        0x003F83F000,  //           #######     ######
        0x007F03F000,  //          #######      ######
        0x00FE03F000,  //         #######       ######
        0x00FC03F000,  //         ######        ######
        0x01F803F000,  //        ######         ######
        0x03F003F000,  //       ######          ######
        0x07F003F000,  //      #######          ######
        0x0FE003F000,  //     #######           ######
        0x1FC003F000,  //    #######            ######
        0x1F8003F000,  //    ######             ######
        0x3F0003F000,  //   ######              ######
        0x7E0003F000,  //  ######               ######
        0xFE0003F000,  // #######               ######
        0xFC0003F000,  // ######                ######
        0xF80003F000,  // #####                 ######
        0xFFFFFFFFE0,  // ###################################
        0xFFFFFFFFE0,  // ###################################
        0xFFFFFFFFE0,  // ###################################
        0xFFFFFFFFE0,  // ###################################
        0xFFFFFFFFE0,  // ###################################
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x000003F000,  //                       ######
        0x0000000000,  //
    },

    [VERDANA_5] = {
        /* @1960 '5' (39 pixels wide) */
        0x03FFFFFFC0,  //       ############################
        0x03FFFFFFC0,  //       ############################
        0x03FFFFFFC0,  //       ############################
        0x03FFFFFFC0,  //       ############################
        0x03FFFFFFC0,  //       ############################
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03F0000000,  //       ######
        0x03FFFE0000,  //       #################
        0x03FFFFC000,  //       ####################
        0x03FFFFF000,  //       ######################
        0x03FFFFF800,  //       #######################
        0x03FFFFFE00,  //       #########################
        0x038007FE00,  //       ###            ##########
        0x000000FF00,  //                         ########
        0x0000007F80,  //                          ########
        0x0000003F80,  //                           #######
        0x0000001F80,  //                            ######
        0x0000001FC0,  //                            #######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000003F80,  //                           #######
        0x0800003F00,  //     #                     ######
        0x0E00007F00,  //     ###                  #######
        0x0FC001FE00,  //     ######             ########
        0x0FF807FC00,  //     #########        #########
        0x0FFFFFF800,  //     #########################
        0x0FFFFFF000,  //     ########################
        0x0FFFFFE000,  //     #######################
        0x01FFFF8000,  //        ##################
        0x001FFC0000,  //            ###########
    },

    [VERDANA_6] = {
        /* @2205 '6' (39 pixels wide) */
        0x00000FFC00,  //                     ##########
        0x0000FFFE00,  //                 ###############
        0x0003FFFE00,  //               #################
        0x000FFFFE00,  //             ###################
        0x001FFFFE00,  //            ####################
        0x003FF00600,  //           ##########         ##
        0x007F800000,  //          ########
        0x00FE000000,  //         #######
        0x01FC000000,  //        #######
        0x03F8000000,  //       #######
        0x03F0000000,  //       ######
        0x07E0000000,  //      ######
        0x07E0000000,  //      ######
        0x0FC0000000,  //     ######
        0x0FC0000000,  //     ######
        0x0F80000000,  //     #####
        0x1F80000000,  //    ######
        0x1F80000000,  //    ######
        0x1F81FF8000,  //    ######      ##########
        0x1F0FFFF000,  //    #####    ################
        0x3F3FFFFC00,  //   ######  ####################
        0x3FFFFFFE00,  //   #############################
        0x3FFFFFFF00,  //   ##############################
        0x3FFC01FF80,  //   ############         ##########
        0x3FE0003FC0,  //   #########               ########
        0x3F00001FC0,  //   ######                   #######
        0x3F00000FE0,  //   ######                    #######
        0x3F000007E0,  //   ######                     ######
        0x3F000007F0,  //   ######                     #######
        0x3F000003F0,  //   ######                      ######
        0x3F000003F0,  //   ######                      ######
        0x3F000003F0,  //   ######                      ######
        0x3F000003F0,  //   ######                      ######
        0x3F000003F0,  //   ######                      ######
        0x1F800003F0,  //    ######                     ######
        0x1F800003F0,  //    ######                     ######
        0x1F800003F0,  //    ######                     ######
        0x1F800007E0,  //    ######                    ######
        0x0FC00007E0,  //     ######                   ######
        0x0FE0000FE0,  //     #######                 #######
        0x07E0000FC0,  //      ######                 ######
        0x07F0001FC0,  //      #######               #######
        0x03FC007F80,  //       ########           ########
        0x01FF01FF00,  //        #########       #########
        0x00FFFFFE00,  //         #######################
        0x007FFFFC00,  //          #####################
        0x003FFFF800,  //           ###################
        0x000FFFE000,  //             ###############
        0x0001FF0000,  //                #########
    },

    [VERDANA_7] = {
        /* @2450 '7' (39 pixels wide) */
        0x0000000000,  //
        0x1FFFFFFFE0,  //    ################################
        0x1FFFFFFFE0,  //    ################################
        0x1FFFFFFFE0,  //    ################################
        0x1FFFFFFFE0,  //    ################################
        0x1FFFFFFFE0,  //    ################################
        0x00000003E0,  //                               #####
        0x00000007E0,  //                              ######
        0x0000000FE0,  //                             #######
        0x0000000FC0,  //                             ######
        0x0000001FC0,  //                            #######
        0x0000001F80,  //                            ######
        0x0000003F80,  //                           #######
        0x0000003F00,  //                           ######
        0x0000007F00,  //                          #######
        0x0000007E00,  //                          ######
        0x000000FE00,  //                         #######
        0x000000FC00,  //                         ######
        0x000001F800,  //                        ######
        0x000003F800,  //                       #######
        0x000003F000,  //                       ######
        0x000007F000,  //                      #######
        0x000007E000,  //                      ######
        0x00000FE000,  //                     #######
        0x00000FC000,  //                     ######
        0x00001FC000,  //                    #######
        0x00001F8000,  //                    ######
        0x00003F8000,  //                   #######
        0x00003F0000,  //                   ######
        0x00007F0000,  //                  #######
        0x0000FE0000,  //                 #######
        0x0000FE0000,  //                 #######
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0003F80000,  //               #######
        0x0003F00000,  //               ######
        0x0007F00000,  //              #######
        0x0007E00000,  //              ######
        0x000FE00000,  //             #######
        0x000FC00000,  //             ######
        0x001FC00000,  //            #######
        0x001F800000,  //            ######
        0x003F800000,  //           #######
        0x007F000000,  //          #######
        0x007F000000,  //          #######
        0x00FE000000,  //         #######
        0x00FE000000,  //         #######
        0x01FC000000,  //        #######
        0x0000000000,  //
    },

    [VERDANA_8] = {
        /* @2695 '8' (39 pixels wide) */
        0x0003FF0000,  //               ##########
        0x001FFFE000,  //            ################
        0x007FFFF800,  //          ####################
        0x01FFFFFC00,  //        #######################
        0x03FFFFFE00,  //       #########################
        0x07FE03FF00,  //      ##########       ##########
        0x07F0007F80,  //      #######             ########
        0x0FE0003F80,  //     #######               #######
        0x0FC0001F80,  //     ######                 ######
        0x1F80001FC0,  //    ######                  #######
        0x1F80000FC0,  //    ######                   ######
        0x1F80000FC0,  //    ######                   ######
        0x1F80000FC0,  //    ######                   ######
        0x1F80000FC0,  //    ######                   ######
        0x1FC0000FC0,  //    #######                  ######
        0x1FC0001F80,  //    #######                 ######
        0x0FE0001F80,  //     #######                ######
        0x0FF0001F00,  //     ########               #####
        0x07FC003F00,  //      #########            ######
        0x03FF007E00,  //       ##########         ######
        0x01FFE1FC00,  //        ############    #######
        0x00FFFFF800,  //         #####################
        0x003FFFE000,  //           #################
        0x007FFFE000,  //          ##################
        0x00FDFFF800,  //         ###### ##############
        0x03F83FFC00,  //       #######     ############
        0x07E007FE00,  //      ######          ##########
        0x07E001FF00,  //      ######            #########
        0x0FC0007F80,  //     ######               ########
        0x1F80003FC0,  //    ######                 ########
        0x1F80000FC0,  //    ######                   ######
        0x3F00000FE0,  //   ######                    #######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F80000FE0,  //   #######                   #######
        0x1F80000FC0,  //    ######                   ######
        0x1FC0001FC0,  //    #######                 #######
        0x0FE0003F80,  //     #######               #######
        0x0FF8007F80,  //     #########            ########
        0x07FE01FF00,  //      ##########        #########
        0x03FFFFFE00,  //       #########################
        0x01FFFFFC00,  //        #######################
        0x00FFFFF000,  //         ####################
        0x003FFFC000,  //           ################
        0x0003FE0000,  //               #########
    },

    [VERDANA_9] = {
        /* @2940 '9' (39 pixels wide) */
        0x0003FE0000,  //               #########
        0x001FFF8000,  //            ##############
        0x007FFFE000,  //          ##################
        0x00FFFFF800,  //         #####################
        0x01FFFFFC00,  //        #######################
        0x03FE07FE00,  //       #########      ##########
        0x07F000FE00,  //      #######            #######
        0x0FE0007F00,  //     #######              #######
        0x0FC0003F80,  //     ######                #######
        0x1FC0001F80,  //    #######                 ######
        0x1F80001F80,  //    ######                  ######
        0x1F80000FC0,  //    ######                   ######
        0x3F00000FC0,  //   ######                    ######
        0x3F00000FC0,  //   ######                    ######
        0x3F000007C0,  //   ######                     #####
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F000007E0,  //   ######                     ######
        0x3F800007E0,  //   #######                    ######
        0x1F800007E0,  //    ######                    ######
        0x1FC00007E0,  //    #######                   ######
        0x0FE00007E0,  //     #######                  ######
        0x0FF0001FE0,  //     ########               ########
        0x07FC01FFE0,  //      #########         ############
        0x03FFFFFFE0,  //       #############################
        0x01FFFFFFE0,  //        ############################
        0x00FFFFE7E0,  //         ###################  ######
        0x003FFF87C0,  //           ###############    #####
        0x0007FC0FC0,  //              #########      ######
        0x0000000FC0,  //                             ######
        0x0000000FC0,  //                             ######
        0x0000000F80,  //                             #####
        0x0000001F80,  //                            ######
        0x0000001F80,  //                            ######
        0x0000001F00,  //                            #####
        0x0000003F00,  //                           ######
        0x0000007E00,  //                          ######
        0x000000FE00,  //                         #######
        0x000001FC00,  //                        #######
        0x000003F800,  //                       #######
        0x000007F800,  //                      ########
        0x01803FF000,  //        ##         ##########
        0x01FFFFE000,  //        ####################
        0x01FFFF8000,  //        ##################
        0x01FFFF0000,  //        #################
        0x01FFF80000,  //        ##############
        0x00FFC00000,  //         ##########
    },

    [VERDANA_PLUS] = {
        /* @0 '+' (39 pixels wide) */
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0xFFFFFFFFFE,  // #######################################
        0xFFFFFFFFFE,  // #######################################
        0xFFFFFFFFFE,  // #######################################
        0xFFFFFFFFFE,  // #######################################
        0xFFFFFFFFFE,  // #######################################
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x00007C0000,  //                  #####
        0x0000000000,  //
        0x0000000000,  //
    },

    [VERDANA_MIN] = {
        /* @245 '-' (39 pixels wide) */
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x007FFFF800,  //          ####################
        0x007FFFF800,  //          ####################
        0x007FFFF800,  //          ####################
        0x007FFFF800,  //          ####################
        0x007FFFF800,  //          ####################
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
    },

    [VERDANA_COMMA] = {
        /* @490 '.' (39 pixels wide) */
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0001FC0000,  //                #######
        0x0000000000,  //
    },

    [VERDANA_DEG] = {
        /* @3185 '°' (39 pixels wide) */
        0x0001FC0000,  //                #######
        0x000FFF8000,  //             #############
        0x001FFFC000,  //            ###############
        0x003FFFE000,  //           #################
        0x007FFFF000,  //          ###################
        0x00FF07F800,  //         ########     ########
        0x01FC01FC00,  //        #######         #######
        0x01F800FC00,  //        ######           ######
        0x01F0007C00,  //        #####             #####
        0x03F0007E00,  //       ######             ######
        0x03E0003E00,  //       #####               #####
        0x03E0003E00,  //       #####               #####
        0x03E0003E00,  //       #####               #####
        0x03E0003E00,  //       #####               #####
        0x03E0003E00,  //       #####               #####
        0x03F0007E00,  //       ######             ######
        0x01F0007C00,  //        #####             #####
        0x01F800FC00,  //        ######           ######
        0x01FC01FC00,  //        #######         #######
        0x00FF07F800,  //         ########     ########
        0x007FFFF000,  //          ###################
        0x003FFFE000,  //           #################
        0x001FFFC000,  //            ###############
        0x000FFF8000,  //             #############
        0x0001FC0000,  //                #######
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
        0x0000000000,  //
    },
        // clang-format on
};
