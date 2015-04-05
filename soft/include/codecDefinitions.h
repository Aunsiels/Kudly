#ifndef CODECDEFINITIONS_H
#define CODECDEFINITIONS_H


/* Macro used for going in data,command or reset mode */

#define RESET_MODE   palSetPad(GPIOE,GPIOE_SPI4_XDCS);palSetPad(GPIOE,GPIOE_SPI4_XCS)
#define COMMAND_MODE palSetPad(GPIOE,GPIOE_SPI4_XDCS);palClearPad(GPIOE,GPIOE_SPI4_XCS)
#define DATA_MODE    palSetPad(GPIOE,GPIOE_SPI4_XCS);palClearPad(GPIOE,GPIOE_SPI4_XDCS)


/* SCI registers */

#define SCI_MODE        0x00
#define SCI_STATUS      0x01
#define SCI_BASS        0x02
#define SCI_CLOCKF      0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA      0x05
#define SCI_WRAM        0x06
#define SCI_WRAMADDR    0x07
#define SCI_HDAT0       0x08 
#define SCI_HDAT1       0x09
#define SCI_AIADDR      0x0A
#define SCI_VOL         0x0B
#define SCI_AICTRL0     0x0C
#define SCI_AICTRL1     0x0D
#define SCI_AICTRL2     0x0E
#define SCI_AICTRL3     0x0F


/* SCI register recording aliases */

#define SCI_RECQUALITY 0x07 /* (WRAMADDR) */
#define SCI_RECDATA    0x08 /* (HDAT0)    */
#define SCI_RECWORDS   0x09 /* (HDAT1)    */
#define SCI_RECRATE    0x0C /* (AICTRL0)  */
#define SCI_RECDIV     0x0C /* (AICTRL0)  */
#define SCI_RECGAIN    0x0D /* (AICTRL1)  */
#define SCI_RECMAXAUTO 0x0E /* (AICTRL2)  */
#define SCI_RECMODE    0x0F /* (AICTRL3)   */


/* SCI_MODE bits */

#define SM_DIFF           (1<< 0)
#define SM_LAYER12        (1<< 1) 
#define SM_RESET          (1<< 2)
#define SM_CANCEL         (1<< 3) 
#define SM_TESTS          (1<< 5)
#define SM_DACT           (1<< 8)
#define SM_SDIORD         (1<< 9)
#define SM_SDISHARE       (1<<10)
#define SM_SDINEW         (1<<11)
#define SM_ENCODE         (1<<12) 
#define SM_LINE1          (1<<14) 
#define SM_CLK_RANGE      (1<<15) 

#define SM_ICONF_BITS 2
#define SM_ICONF_MASK 0x00c0

#define SM_EARSPEAKER_1103_BITS 2
#define SM_EARSPEAKER_1103_MASK 0x3000


/* SCI_STATUS bits */

#define SS_REFERENCE_SEL (1<< 0)
#define SS_AD_CLOCK      (1<< 1)
#define SS_APDOWN1       (1<< 2)
#define SS_APDOWN2       (1<< 3)
#define SS_VER           (1<< 4)
#define SS_VCM_DISABLE   (1<<10)
#define SS_VCM_OVERLOAD  (1<<11)
#define SS_SWING         (1<<12)
#define SS_DO_NOT_JUMP   (1<<15)

#define SS_SWING_BITS     3
#define SS_SWING_MASK     0x7000
#define SS_VER_BITS       4
#define SS_VER_MASK       0x00f0
#define SS_AVOL_BITS      2
#define SS_AVOL_MASK      0x0003

#define SS_VER_VS1063 0x60


/* SCI_BASS bits */

#define ST_AMPLITUDE (1<<12)
#define ST_FREQLIMIT (1<< 8)
#define SB_AMPLITUDE (1<< 4)
#define SB_FREQLIMIT (1<< 0)

#define ST_AMPLITUDE_BITS 4
#define ST_AMPLITUDE_MASK 0xf000
#define ST_FREQLIMIT_BITS 4
#define ST_FREQLIMIT_MASK 0x0f00
#define SB_AMPLITUDE_BITS 4
#define SB_AMPLITUDE_MASK 0x00f0
#define SB_FREQLIMIT_BITS 4
#define SB_FREQLIMIT_MASK 0x000f


/* SCI_CLOCKF bits */

#define SC_MULT (1<<13) 
#define SC_ADD  (1<<11) 
#define SC_FREQ (1<< 0) 

#define SC_MULT_BITS 3
#define SC_MULT_MASK 0xe000
#define SC_ADD_BITS 2
#define SC_ADD_MASK  0x1800
#define SC_FREQ_BITS 11
#define SC_FREQ_MASK 0x07ff

#define SC_MULT_53_10X 0x0000
#define SC_MULT_53_20X 0x2000
#define SC_MULT_53_25X 0x4000
#define SC_MULT_53_30X 0x6000
#define SC_MULT_53_35X 0x8000
#define SC_MULT_53_40X 0xa000
#define SC_MULT_53_45X 0xc000
#define SC_MULT_53_50X 0xe000

#define SC_ADD_53_00X 0x0000
#define SC_ADD_53_10X 0x0800
#define SC_ADD_53_15X 0x1000
#define SC_ADD_53_20X 0x1800


/* SCI_WRAMADDR bits */

#define SCI_WRAM_X_START           0x0000
#define SCI_WRAM_Y_START           0x4000
#define SCI_WRAM_I_START           0x8000
#define SCI_WRAM_IO_START          0xC000
#define SCI_WRAM_PARAMETRIC_START  0xC0C0 
#define SCI_WRAM_Y2_START          0xE000 

#define SCI_WRAM_X_OFFSET  0x0000
#define SCI_WRAM_Y_OFFSET  0x4000
#define SCI_WRAM_I_OFFSET  0x8000
#define SCI_WRAM_IO_OFFSET 0x0000 /* I/O addresses are @0xC000 -> no offset */
#define SCI_WRAM_PARAMETRIC_OFFSET (0xC0C0-0x1E00) 
#define SCI_WRAM_Y2_OFFSET 0x0000                  


/* SCI_VOL bits */

#define SV_LEFT_B  8
#define SV_RIGHT_B 0

#define SV_LEFT  (1<<8)
#define SV_RIGHT (1<<0)

#define SV_LEFT_BITS  8
#define SV_LEFT_MASK  0xFF00
#define SV_RIGHT_BITS 8
#define SV_RIGHT_MASK 0x00FF


/* SCI_RECQUALITY bits */

#define RQ_MODE                   (1<<14)
#define RQ_MULT                   (1<<12)
#define RQ_OGG_PAR_SERIAL_NUMBER  (1<<11)
#define RQ_OGG_LIMIT_FRAME_LENGTH (1<<10)
#define RQ_MP3_NO_BIT_RESERVOIR   (1<<10)
#define RQ_BITRATE_BASE           (1<< 0)

#define RQ_MODE_BITS 2
#define RQ_MODE_MASK 0xc000
#define RQ_MULT_BITS 2
#define RQ_MULT_MASK 0x3000
#define RQ_BITRATE_BASE_BITS 9
#define RQ_BITRATE_BASE_MASK 0x01ff

#define RQ_MODE_QUALITY 0x0000
#define RQ_MODE_VBR     0x4000
#define RQ_MODE_ABR     0x8000
#define RQ_MODE_CBR     0xc000

#define RQ_MULT_10      0x0000
#define RQ_MULT_100     0x1000
#define RQ_MULT_1000    0x2000
#define RQ_MULT_10000   0x3000


/* SCI_RECMODE bits for */

#define RM_63_CODEC    (1<<15)
#define RM_63_AEC      (1<<14)
#define RM_63_UART_TX  (1<<13)
#define RM_63_PAUSE    (1<<11)
#define RM_63_NO_RIFF  (1<<10)
#define RM_63_FORMAT   (1<< 4)
#define RM_63_ADC_MODE (1<< 0)

#define RM_63_FORMAT_BITS       4
#define RM_63_FORMAT_MASK  0x00f0
#define RM_63_ADCMODE_BITS      3
#define RM_63_ADCMODE_MASK 0x0007

#define RM_63_FORMAT_IMA_ADPCM  0x0000
#define RM_63_FORMAT_PCM        0x0010
#define RM_63_FORMAT_G711_ULAW  0x0020
#define RM_63_FORMAT_G711_ALAW  0x0030
#define RM_63_FORMAT_G722_ADPCM 0x0040
#define RM_63_FORMAT_OGG_VORBIS 0x0050
#define RM_63_FORMAT_MP3        0x0060

#define RM_63_ADC_MODE_JOINT_AGC_STEREO 0x0000
#define RM_63_ADC_MODE_DUAL_AGC_STEREO  0x0001
#define RM_63_ADC_MODE_LEFT             0x0002
#define RM_63_ADC_MODE_RIGHT            0x0003
#define RM_63_ADC_MODE_MONO             0x0004

/* Parametric */
#define PAR_CHIP_ID                  0x1e00 /* 32 bits */
#define PAR_VERSION                  0x1e02 
#define PAR_CONFIG1                  0x1e03 
#define PAR_PLAY_SPEED               0x1e04 
#define PAR_BITRATE_PER_100          0x1e05 
#define PAR_END_FILL_BYTE            0x1e06 
#define PAR_RATE_TUNE                0x1e07 /* 32 bits */
#define PAR_PLAY_MODE                0x1e09 
#define PAR_SAMPLE_COUNTER           0x1e0a /* 32 bits */
#define PAR_VU_METER                 0x1e0c 
#define PAR_AD_MIXER_GAIN            0x1e0d 
#define PAR_AD_MIXER_CONFIG          0x1e0e 
#define PAR_PCM_MIXER_RATE           0x1e0f 
#define PAR_PCM_MIXER_FREE           0x1e10 
#define PAR_PCM_MIXER_VOL            0x1e11 
#define PAR_EQ5_DUMMY                0x1e12 
#define PAR_EQ5_LEVEL1               0x1e13 
#define PAR_EQ5_FREQ1                0x1e14 
#define PAR_EQ5_LEVEL2               0x1e15 
#define PAR_EQ5_FREQ2                0x1e16 
#define PAR_EQ5_LEVEL3               0x1e17 
#define PAR_EQ5_FREQ3                0x1e18 
#define PAR_EQ5_LEVEL4               0x1e19 
#define PAR_EQ5_FREQ4                0x1e1a 
#define PAR_EQ5_LEVEL5               0x1e1b 
#define PAR_EQ5_UPDATED              0x1e1c 
#define PAR_SPEED_SHIFTER            0x1e1d 
#define PAR_EARSPEAKER_LEVEL         0x1e1e 
#define PAR_SDI_FREE                 0x1e1f 
#define PAR_AUDIO_FILL               0x1e20 
#define PAR_RESERVED0                0x1e21 
#define PAR_RESERVED1                0x1e22 
#define PAR_RESERVED2                0x1e23 
#define PAR_RESERVED3                0x1e24 
#define PAR_LATEST_SOF               0x1e25 /* 32 bits */
#define PAR_POSITION_MSEC            0x1e27 /* 32 bits */
#define PAR_RESYNC                   0x1e29 

/* The following addresses are shared between modes. */
/* Generic pointer */
#define PAR_GENERIC                  0x1e2a 

/* Encoder mode */
#define PAR_ENC_TX_UART_DIV          0x1e2a 
#define PAR_ENC_TX_UART_BYTE_SPEED   0x1e2b 
#define PAR_ENC_TX_PAUSE_GPIO        0x1e2c 
#define PAR_ENC_AEC_ADAPT_MULTIPLIER 0x1e2d 
#define PAR_ENC_RESERVED             0x1e2e 
#define PAR_ENC_CHANNEL_MAX          0x1e3c 
#define PAR_ENC_SERIAL_NUMBER        0x1e3e 

/* Decoding WMA */
#define PAR_WMA_CUR_PACKET_SIZE      0x1e2a /* 32 bits */
#define PAR_WMA_PACKET_SIZE          0x1e2c /* 32 bits */

/* Decoding AAC */
#define PAR_AAC_SCE_FOUND_MASK       0x1e2a 
#define PAR_AAC_CPE_FOUND_MASK       0x1e2b 
#define PAR_AAC_LFE_FOUND_MASK       0x1e2c 
#define PAR_AAC_PLAY_SELECT          0x1e2d 
#define PAR_AAC_DYN_COMPRESS         0x1e2e 
#define PAR_AAC_DYN_BOOST            0x1e2f 
#define PAR_AAC_SBR_AND_PS_STATUS    0x1e30 
#define PAR_AAC_SBR_PS_FLAGS         0x1e31 

/* Decoding Vorbis */
#define PAR_VORBIS_GAIN 0x1e2a       0x1e30 


/* Bit definitions for parametric registers with bitfields */

#define PAR_CONFIG1_DIS_WMA     (1<<15) 
#define PAR_CONFIG1_DIS_AAC     (1<<14) 
#define PAR_CONFIG1_DIS_MP3     (1<<13) 
#define PAR_CONFIG1_DIS_FLAC    (1<<12) 
#define PAR_CONFIG1_DIS_CRC     (1<< 8) 
#define PAR_CONFIG1_AAC_PS      (1<< 6) 
#define PAR_CONFIG1_AAC_SBR     (1<< 4) 

#define PAR_CONFIG1_AAC_PS_BITS  2      
#define PAR_CONFIG1_AAC_PS_MASK  0x00c0 
#define PAR_CONFIG1_AAC_SBR_BITS 2      
#define PAR_CONFIG1_AAC_SBR_MASK 0x0030 

#define PAR_CONFIG1_AAC_SBR_ALWAYS_UPSAMPLE    0x0000 
#define PAR_CONFIG1_AAC_SBR_SELECTIVE_UPSAMPLE 0x0010 
#define PAR_CONFIG1_AAC_SBR_NEVER_UPSAMPLE     0x0020 
#define PAR_CONFIG1_AAC_SBR_DISABLE            0x0030 

#define PAR_CONFIG1_AAC_PS_NORMAL              0x0000 
#define PAR_CONFIG1_AAC_PS_DOWNSAMPLED         0x0040 
#define PAR_CONFIG1_AAC_PS_DISABLE             0x00c0 

#define PAR_PLAY_MODE_SPEED_SHIFTER_ENA (1<<6) 
#define PAR_PLAY_MODE_EQ5_ENA           (1<<5) 
#define PAR_PLAY_MODE_PCM_MIXER_ENA     (1<<4) 
#define PAR_PLAY_MODE_AD_MIXER_ENA      (1<<3) 
#define PAR_PLAY_MODE_VU_METER_ENA      (1<<2) 
#define PAR_PLAY_MODE_PAUSE_ENA         (1<<1) 
#define PAR_PLAY_MODE_MONO_ENA          (1<<0) 

#define PAR_VU_METER_LEFT_BITS  8      
#define PAR_VU_METER_LEFT_MASK  0xFF00 
#define PAR_VU_METER_RIGHT_BITS 8      
#define PAR_VU_METER_RIGHT_MASK 0x00FF 

#define PAR_AD_MIXER_CONFIG_MODE_B 2 
#define PAR_AD_MIXER_CONFIG_RATE_B 2 

#define PAR_AD_MIXER_CONFIG_MODE_BITS 2      
#define PAR_AD_MIXER_CONFIG_MODE_MASK 0x000c 
#define PAR_AD_MIXER_CONFIG_RATE_BITS 2      
#define PAR_AD_MIXER_CONFIG_RATE_MASK 0x0003 

#define PAR_AD_MIXER_CONFIG_RATE_192K 0x0000 
#define PAR_AD_MIXER_CONFIG_RATE_96K  0x0001 
#define PAR_AD_MIXER_CONFIG_RATE_48K  0x0002 
#define PAR_AD_MIXER_CONFIG_RATE_24K  0x0003 

#define PAR_AD_MIXER_CONFIG_MODE_STEREO 0x0000 
#define PAR_AD_MIXER_CONFIG_MODE_MONO   0x0040 
#define PAR_AD_MIXER_CONFIG_MODE_LEFT   0x0080 
#define PAR_AD_MIXER_CONFIG_MODE_RIGHT  0x00c0 

#define PAR_AAC_SBR_AND_PS_STATUS_SBR_PRESENT       (1<<0) 
#define PAR_AAC_SBR_AND_PS_STATUS_UPSAMPLING_ACTIVE (1<<1) 
#define PAR_AAC_SBR_AND_PS_STATUS_PS_PRESENT        (1<<2) 
#define PAR_AAC_SBR_AND_PS_STATUS_PS_ACTIVE         (1<<3) 

#endif /* CODEC_DEFINITIONS_H */
