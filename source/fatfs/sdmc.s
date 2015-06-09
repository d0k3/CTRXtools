@ ---------------------------------------------------------------------------
	@.section	.itcm,"ax",%progbits 
	
.thumb

.global sdmmc_send_command

@ int __cdecl sdmmc_send_command(unsigned int *ctx, unsigned int cmd, unsigned int args)
sdmmc_send_command:                     @ CODE XREF: SD_Init+18p
                                        @ SD_Init+24p ...
                PUSH    {R0-R2,R4-R7,LR}
                MOV     R5, #0
                STR     R5, [R0,#8]
                LDR     R4, [R0,#4]
                MOV     R5, #0x80
                LDR     R3, [R0]
                LDR     R7, =0x10006000
                LSL     R5, R5, #7

loc_80A3700:                            @ CODE XREF: ROM:080A3704j
                LDRH    R6, [R7, #0x1E]
                AND     R6, R5
                BNE     loc_80A3700
                LDR     R5, =0x10006000
                STRH    R6, [R7, #0x20]
                STRH    R6, [R7, #0x22]
                STRH    R6, [R7, #0x1C]
                STRH    R6, [R7, #0x1E]
                LDR     R6, =0x10006100
                LDR     R7, =0xFFFFE7FF
                LDRH    R5, [R6]
                AND     R7, R5
                STRH    R7, [R6]
                LSL     R6, R2, #0x10
                LDR     R7, =0x10006000
                LSR     R6, R6, #0x10
                STRH    R6, [R7, #4]
                LSR     R2, R2, #0x10
                STRH    R2, [R7,#6]
                LSL     R2, R1, #0x10
                LSR     R2, R2, #0x10
                STRH    R2, [R7]
                MOV     R5, #0xC0
                LSL     R6, R1, #0xF
                LSR     R6, R6, #0x1F
                MOV     R7, #4
                LSL     R5, R5, #0xB
                ORR     R7, R6
                MOV     R2, R6
                TST     R1, R5
                BEQ     loc_80A374A
                ADD     R2, R7, #0

loc_80A374A:                            @ CODE XREF: ROM:080A3746j
                LSL     R2, R2, #0x10
                LSR     R2, R2, #0x10
                MOV     R12, R2
                @MOV     R2, 0x20000
				MOV     R2, #0x80
				LSL     R2, R2, #0xA
                AND     R2, R1
                STR     R2, [SP]
                @MOV     R2, 0x40000
				MOV     R2, #0x80
				LSL     R2, R2, #0xB
                AND     R1, R2
                STR     R1, [SP,#4]

loc_80A3760:                            @ CODE XREF: ROM:080A37D8j
                                        @ ROM:080A37F8j
                LDR     R7, =0x1000601E
                LDRH    R2, [R7]
                LSL     R1, R2, #0x17
                BPL     loc_80A3792
                LDR     R5, [SP]
                CMP     R5, #0
                BEQ     loc_80A3792
                CMP     R3, #0
                BEQ     loc_80A3792
                LDR     R5, =0xFFFFFEFF
                LDR     R1, =0x1FF
                STRH    R5, [R7]
                CMP     R4, R1
                BLS     loc_80A3792
                @MOV     R7, 0x200
				MOV     R7, #0x80
				LSL     R7, R7, #2
                ADD     R1, R3, R7
                LDR     R7, =0x10006030

loc_80A3784:                            @ CODE XREF: ROM:080A378Cj
                LDRH    R5, [R7]
                STRH    R5, [R3]
                ADD     R3, #2
                CMP     R3, R1
                BNE     loc_80A3784
                LDR     R1, =0xFFFFFE00
                ADD     R4, R4, R1

loc_80A3792:                            @ CODE XREF: ROM:080A3766j
                                        @ ROM:080A376Cj ...
                LSL     R5, R2, #0x16
                BPL     loc_80A37C2
                LDR     R7, [SP,#4]
                CMP     R7, #0
                BEQ     loc_80A37C2
                CMP     R3, #0
                BEQ     loc_80A37C2
                LDR     R1, =0xFFFFFDFF
                LDR     R5, =0x1000601E
                STRH    R1, [R5]
                LDR     R1, =0x1FF
                CMP     R4, R1
                BLS     loc_80A37C2
                @MOV     R7, 0x200
				MOV     R7, #0x80
				LSL     R7, R7, #2
                LDR     R5, =0x10006030
                ADD     R1, R3, R7

loc_80A37B4:                            @ CODE XREF: ROM:080A37BCj
                LDRH    R7, [R3]
                ADD     R3, #2
                STRH    R7, [R5]
                CMP     R3, R1
                BNE     loc_80A37B4
                LDR     R1, =0xFFFFFE00
                ADD     R4, R4, R1

loc_80A37C2:                            @ CODE XREF: ROM:080A3794j
                                        @ ROM:080A379Aj ...
                LDR     R1, =0xFFFF807F
                TST     R2, R1
                BEQ     loc_80A37D2
                LDR     R2, [R0,#8]
                MOV     R3, #4
                ORR     R3, R2
                STR     R3, [R0,#8]
                B       loc_80A37FA
@ ---------------------------------------------------------------------------

loc_80A37D2:                            @ CODE XREF: ROM:080A37C6j
                LDR     R5, =0x1000601C
                LDRH    R1, [R5]
                LSL     R7, R2, #0x11
                BMI     loc_80A3760
                MOV     R2, #1
                TST     R1, R2
                BEQ     loc_80A37E6
                LDR     R5, [R0,#8]
                ORR     R2, R5
                STR     R2, [R0,#8]

loc_80A37E6:                            @ CODE XREF: ROM:080A37DEj
                LSL     R7, R1, #0x1D
                BPL     loc_80A37F2
                LDR     R5, [R0,#8]
                 MOV     R2, #2
                ORR     R2, R5
                STR     R2, [R0,#8]

loc_80A37F2:                            @ CODE XREF: ROM:080A37E8j
                MOV     R7, R12
                AND     R1, R7
                CMP     R1, R12
                BNE     loc_80A3760

loc_80A37FA:                            @ CODE XREF: ROM:080A37D0j
                LDR     R1, =0x1000601C
                MOV     R3, #0
                STRH    R3, [R1]
                STRH    R3, [R1,#2]
                CMP     R6, R3
                BEQ     locret_80A3840
                LDR     R5, =0x10006000
                LDRH    R2, [R5,#0x0C]
                LDRH    R3, [R5,#0x0E]
                LSL     R3, R3, #0x10
                ORR     R3, R2
                STR     R3, [R0,#0xC]
                LDRH    R2, [R5,#0x10]
                LDRH    R3, [R5,#0x12]
                LSL     R3, R3, #0x10
                ORR     R3, R2
                STR     R3, [R0,#0x10]
                LDRH    R2, [R5,#0x14]
                LDRH    R3, [R5,#0x16]
                LSL     R3, R3, #0x10
                ORR     R3, R2
                STR     R3, [R0,#0x14]
                LDRH    R2, [R5,#0x18]
                LDRH    R3, [R5,#0x1A]
                LSL     R3, R3, #0x10
                ORR     R3, R2
                STR     R3, [R0,#0x18]

locret_80A3840:                         @ CODE XREF: ROM:080A3806j
                POP     {R0-R2,R4-R7,PC}
@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================


sub_80A38A0:                            @ CODE XREF: sub_80A38D4+14p
                                        @ SD_Init+B6p ...
                LDR     R3, =0x10006024
                LDR     R2, =0xFFFFFEFF
                LDRH    R1, [R3]
                AND     R2, R1
                STRH    R2, [R3]
                LDRH    R1, [R3]
                LDR     R2, =0xFFFFFD00
                AND     R2, R1
                LDR     R1, =0x2FF
                AND     R0, R1
                ORR     R0, R2
                STRH    R0, [R3]
                LDRH    R1, [R3]
                MOV     R2, #0x80
                LSL     R2, R2, #1
                ORR     R2, R1
                STRH    R2, [R3]
                BX      LR
@ End of function sub_80A38A0

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================


sub_80A38D4:                            @ CODE XREF: InitSD+C4p
                                        @ SD_Init+6p ...
                PUSH    {R4,LR}
                LDR     R3, =0x10006002
                MOV     R1, #3
                LDRH    R2, [R3]
                ADD     R4, R0, #0
                BIC     R2, R1
                LDRH    R1, [R0,#0x2C]
                ORR     R2, R1
                STRH    R2, [R3]
                LDR     R0, [R0,#0x24]
                BL      sub_80A38A0
                LDR     R2, [R4,#0x28]
                LDR     R3, =0x10006028
                CMP     R2, #0
                BNE     loc_80A38FE
                LDRH    R1, [R3]
                MOV     R2, #0x80
                LSL     R2, R2, #8
                ORR     R2, R1
                B       loc_80A3904
@ ---------------------------------------------------------------------------

loc_80A38FE:                            @ CODE XREF: sub_80A38D4+1Ej
                LDRH    R2, [R3]
                LSL     R2, R2, #0x11
                LSR     R2, R2, #0x11

loc_80A3904:                            @ CODE XREF: sub_80A38D4+28j
                STRH    R2, [R3]
                POP     {R4,PC}
@ End of function sub_80A38D4

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================


sub_80A3910:                            @ CODE XREF: SD_Init+Ep
                                        @ Nand_Init+Ep

                PUSH    {R0-R2,LR}
                LSL     R0, R0, #2
                STR     R0, [SP,#4]

loc_80A3916:                            @ CODE XREF: sub_80A3910+Ej
                LDR     R3, [SP,#4]
                SUB     R2, R3, #1
                STR     R2, [SP,#4]
                CMP     R3, #0
                BNE     loc_80A3916
                POP     {R0-R2,PC}
@ End of function sub_80A3910

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global InitSD

InitSD:                                 @ CODE XREF: sub_80A244C+Ep
                PUSH    {R3-R7,LR}
                LDR     R2, =dword_80A6F20
                MOV     R3, #0
                LDR     R0, =dword_80A6F58
                MOV     R1, #1
                MOV     R4, #0x80
                STR     R3, [R2,#0x20]
                STR     R3, [R2,#0x28]
                STR     R3, [R2,#0x34]
                STRH    R1, [R2,#0x1C]
                STR     R4, [R2,#0x24]
                STR     R1, [R2,#0x2C]
                LDR     R2, =0x10006100
                STR     R4, [R0,#0x24]
                STRH    R3, [R0,#0x1C]
                STR     R3, [R0,#0x20]
                STR     R3, [R0,#0x28]
                STR     R3, [R0,#0x2C]
                STR     R3, [R0,#0x34]
                LDRH    R5, [R2]
                LDR     R4, =0xFFFFF7FF
                MOV     R6, #2
                AND     R4, R5
                STRH    R4, [R2]
                LDRH    R5, [R2]
                LDR     R4, =0xFFFFEFFF
                AND     R4, R5
                STRH    R4, [R2]
                LDRH    R4, [R2]
                LDR     R5, =0x402
                ORR     R4, R5
                STRH    R4, [R2]
                LDR     R4, =0x100060D8
                MOV     R5, #0x22
                LDRH    R7, [R4]
                BIC     R7, R5
                ORR     R7, R6
                STRH    R7, [R4]
                LDRH    R7, [R2]
                BIC     R7, R6
                STRH    R7, [R2]
                LDRH    R2, [R4]
                BIC     R2, R5
                STRH    R2, [R4]
                LDR     R2, =0x10006104
                LDR     R5, =0x10006024
                STRH    R3, [R2]
                LDR     R2, =0x10006108
                STRH    R1, [R2]
                LDR     R2, =0x100060E0
                LDRH    R4, [R2]
                BIC     R4, R1
                STRH    R4, [R2]
                LDRH    R4, [R2]
                ORR     R1, R4
                STRH    R1, [R2]
                LDR     R2, =0x10006020
                LDR     R4, =0x31D
                LDRH    R1, [R2]
                ORR     R1, R4
                STRH    R1, [R2]
                LDR     R2, =0x10006022
                LDR     R1, =0x837F
                LDRH    R4, [R2]
                ORR     R1, R4
                STRH    R1, [R2]
                LDR     R2, =0x100060FC
                MOV     R1, #0xDB
                LDRH    R4, [R2]
                ORR     R4, R1
                STRH    R4, [R2]
                LDR     R2, =0x100060FE
                LDRH    R4, [R2]
                ORR     R1, R4
                STRH    R1, [R2]
                MOV     R2, #0x20
                STRH    R2, [R5]
                LDR     R4, =0x10006028
                LDR     R2, =0x40EE
                MOV     R1, #3
                STRH    R2, [R4]
                LDR     R2, =0x10006002
                LDRH    R6, [R2]
                BIC     R6, R1
                STRH    R6, [R2]
                MOV     R6, #0x40
                STRH    R6, [R5]
                LDR     R5, =0x40EB
                STRH    R5, [R4]
                LDRH    R4, [R2]
                BIC     R4, R1
                STRH    R4, [R2]
                MOV     R1, #0x80
                LDR     R2, =0x10006026
                LSL     R1, R1, #2
                STRH    R1, [R2]
                LDR     R2, =0x10006008
                STRH    R3, [R2]
                BL      sub_80A38D4
                POP     {R3-R7,PC}
@ End of function InitSD

@ ---------------------------------------------------------------------------
.pool

sub_80A3A4C:                            @ CODE XREF: sdmmc_sdcard_readsectors+3Ap
                                        @ sdmmc_sdcard_writesectors+42p ...
                LDR     R0, [R0,#8]
                LSL     R0, R0, #0x1D
                ASR     R0, R0, #0x1F
                BX      LR
@ End of function sub_80A3A4C
.pool

@ =============== S U B R O U T I N E =======================================


sub_80A3A54:                            @ CODE XREF: SD_Init+AAp
                                        @ Nand_Init+78p
                PUSH    {R4-R6,LR}
                ADD     R4, R0, #0
                LDRB    R3, [R0,#0xE]
                ADD     R2, R1, #1
                BNE     loc_80A3A60
                LSR     R1, R3, #6

loc_80A3A60:                            @ CODE XREF: sub_80A3A54+8j
                CMP     R1, #0
                BNE     loc_80A3AB0
                LDRB    R6, [R4,#7]
                LDRB    R0, [R4,#8]
                LSL     R6, R6, #2
                LSL     R0, R0, #0xA
                ORR     R0, R6
                LDRB    R6, [R4,#6]
                LDRB    R2, [R4,#9]
                LSR     R6, R6, #6
                MOV     R5, #1
                MOV     R3, #0xF
                ORR     R0, R6
                AND     R2, R3
                LSL     R0, R0, #0x14
                ADD     R3, R5, #0
                LSL     R3, R2
                LSR     R0, R0, #0x14
                ADD     R2, R3, #0
                ADD     R0, R0, R5
                ASR     R3, R3, #0x1F
                BL      sub_80A6754
                LDRB    R2, [R4,#4]
                LDRB    R3, [R4,#5]
                LSR     R2, R2, #7
                LSL     R3, R5
                ORR     R2, R3
                MOV     R3, #7
                AND     R2, R3
                ADD     R2, #2
                LSL     R5, R2
                ADD     R2, R5, #0
                ASR     R3, R5, #0x1F
                BL      sub_80A6754
                LSL     R1, R1, #0x17
                LSR     R0, R0, #9
                ORR     R0, R1
                B       locret_80A3ACC
@ ---------------------------------------------------------------------------

loc_80A3AB0:                            @ CODE XREF: sub_80A3A54+Ej
                MOV     R0, #0
                CMP     R1, #1
                BNE     locret_80A3ACC
                LDRB    R0, [R4,#6]
                LDRB    R3, [R4,#5]
                LSL     R0, R0, #8
                LDRB    R2, [R4,#7]
                ORR     R0, R3
                MOV     R3, #0x3F
                AND     R3, R2
                LSL     R3, R3, #0x10
                ORR     R0, R3
                ADD     R0, #1
                LSL     R0, R0, #0xA

locret_80A3ACC:                         @ CODE XREF: sub_80A3A54+5Aj
                                        @ sub_80A3A54+60j
                POP     {R4-R6,PC}
@ End of function sub_80A3A54

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global SD_Init

SD_Init:                                @ CODE XREF: sub_80A244C+16p
                PUSH    {R4-R6,LR}
                LDR     R4, =dword_80A6F58
                ADD     R0, R4, #0
                BL      sub_80A38D4
                MOV     R0, #0xF0
                LSL     R0, R0, #8
                BL      sub_80A3910
                MOV     R1, #0          @ cmd
                ADD     R2, R1, #0      @ args
                ADD     R0, R4, #0      @ ctx
                BL      sdmmc_send_command
                MOV     R2, #0xD5
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x10408    @ cmd
                LSL     R2, R2, #1
                BL      sdmmc_send_command
                LDR     R5, [R4,#8]
                MOV     R3, #1
                AND     R5, R3
                LSL     R5, R5, #0x1E

loc_80A3B00:                            @ CODE XREF: SD_Init+50j
                                        @ SD_Init+56j
                LDR     R4, =dword_80A6F58
                LDR     R1, =0x10437    @ cmd
                LDRH    R2, [R4,#0x1C]
                ADD     R0, R4, #0      @ ctx
                LSL     R2, R2, #0x10
                BL      sdmmc_send_command
                LDR     R2, =0xFF8000
                ADD     R0, R4, #0      @ ctx
                ORR     R2, R5          @ args
                LDR     R1, =0x10769    @ cmd
                BL      sdmmc_send_command
                LDR     R2, [R4,#8]
                MOV     R3, #1
                TST     R2, R3
                BEQ     loc_80A3B00
                LDR     R2, [R4,#0xC]
                CMP     R2, #0
                BGE     loc_80A3B00
                LSR     R2, R2, #0x1E
                TST     R2, R3
                BEQ     loc_80A3B34
                CMP     R5, #0
                BEQ     loc_80A3B34
                B       loc_80A3B36
@ ---------------------------------------------------------------------------

loc_80A3B34:                            @ CODE XREF: SD_Init+5Cj
                                        @ SD_Init+60j
                MOV     R3, #0

loc_80A3B36:                            @ CODE XREF: SD_Init+62j
                MOV     R2, #0          @ args
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x10602    @ cmd
                STR     R3, [R4,#0x20]
                BL      sdmmc_send_command
                LDR     R2, [R4,#8]
                MOV     R5, #4
                AND     R2, R5
                BEQ     loc_80A3B50

loc_80A3B4A:                            @ CODE XREF: SD_Init+8Cj
                                        @ SD_Init+A0j ...
                MOV     R0, #1
                NEG     R0, R0
                B       locret_80A3BEE
@ ---------------------------------------------------------------------------

loc_80A3B50:                            @ CODE XREF: SD_Init+78j
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x10403    @ cmd
                BL      sdmmc_send_command
                LDR     R3, [R4,#8]
                TST     R3, R5
                BNE     loc_80A3B4A
                LDRH    R2, [R4,#0xE]
                ADD     R0, R4, #0      @ ctx
                STRH    R2, [R4,#0x1C]
                LDR     R1, =0x10609    @ cmd
                LSL     R2, R2, #0x10
                BL      sdmmc_send_command
                LDR     R2, [R4,#8]
                TST     R2, R5
                BNE     loc_80A3B4A
                ADD     R0, R4, #0
                MOV     R1, #1
                NEG     R1, R1
                ADD     R0, #0xC
                BL      sub_80A3A54
                MOV     R6, #1
                STR     R0, [R4,#0x30]
                ADD     R0, R6, #0
                STR     R6, [R4,#0x24]
                BL      sub_80A38A0
                LDRH    R2, [R4,#0x1C]
                ADD     R0, R4, #0      @ ctx
                LSL     R2, R2, #0x10
                LDR     R1, =0x10507    @ cmd
                BL      sdmmc_send_command
                LDR     R3, [R4,#8]
                TST     R3, R5
                BNE     loc_80A3B4A
                LDRH    R2, [R4,#0x1C]
                ADD     R0, R4, #0      @ ctx
                LSL     R2, R2, #0x10
                LDR     R1, =0x10437    @ cmd
                BL      sdmmc_send_command
                LDR     R2, [R4,#8]
                TST     R2, R5
                BNE     loc_80A3B4A
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x10446    @ cmd
                MOV     R2, #2          @ args
                STR     R6, [R4,#0x28]
                BL      sdmmc_send_command
                LDR     R3, [R4,#8]
                TST     R3, R5
                BNE     loc_80A3B4A
                LDRH    R2, [R4,#0x1C]
                ADD     R0, R4, #0      @ ctx
                LSL     R2, R2, #0x10
                LDR     R1, =0x1040D    @ cmd
                BL      sdmmc_send_command
                LDR     R2, [R4,#8]
                TST     R2, R5
                BNE     loc_80A3B4A
                MOV     R6, #0x80
                LSL     R6, R6, #2
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x10410    @ cmd
                ADD     R2, R6, #0      @ args
                BL      sdmmc_send_command
                LDR     R3, [R4,#8]
                AND     R5, R3
                BNE     loc_80A3B4A
                LDR     R2, [R4,#0x24]
                ADD     R0, R5, #0
                ORR     R6, R2
                STR     R6, [R4,#0x24]

locret_80A3BEE:                         @ CODE XREF: SD_Init+7Ej
                POP     {R4-R6,PC}
@ End of function SD_Init

@ ---------------------------------------------------------------------------
.pool

sub_80A3C20:                            @ CODE XREF: sub_80A3EC0+1Ap
                LDR     R3, =dword_80A6F58
                LDR     R0, [R3,#0x30]
                BX      LR
@ End of function sub_80A3C20

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global sdmmc_sdcard_readsector

sdmmc_sdcard_readsector:
                PUSH    {R4-R6,LR}
                LDR     R4, =dword_80A6F58
                ADD     R5, R0, #0
                LDR     R3, [R4,#0x20]
                ADD     R6, R1, #0
                CMP     R3, #0
                BNE     sdmmc_sdcard_readsector_
                LSL     R5, R0, #9 @SDMC works with raw addresses not sectors

sdmmc_sdcard_readsector_:
                BL      sub_80A38D4
                MOV     R3, #0x80
                LSL     R3, R3, #2 @ 0x200
                ADD     R2, R5, #0      @ args
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x31C11    @ cmd
                STR     R6, [R4]
                STR     R3, [R4,#4]
                BL      sdmmc_send_command
                ADD     R0, R4, #0
                BL      sub_80A3A4C
                POP     {R4-R6,PC}
@ End of function sdmmc_sdcard_readsectors

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global sdmmc_sdcard_readsectors

sdmmc_sdcard_readsectors:                      @ CODE XREF: sub_80A3E74+1Cp
                PUSH    {R3-R7,LR}
                LDR     R4, =dword_80A6F58
                ADD     R5, R0, #0
                LDR     R3, [R4,#0x20]
                ADD     R7, R2, #0
                ADD     R6, R1, #0
                CMP     R3, #0
                BNE     loc_80A3C3E
                LSL     R5, R0, #9

loc_80A3C3E:                            @ CODE XREF: sdmmc_sdcard_readsectors+Ej
                ADD     R0, R4, #0
                BL      sub_80A38D4
                LDR     R3, =0x10006008
                MOV     R2, #0x80
                LSL     R2, R2, #1
                STRH    R2, [R3]
                LSL     R3, R6, #0x10
                LDR     R2, =0x1000600A
                LSR     R3, R3, #0x10
                STRH    R3, [R2]
                LSL     R6, R6, #9
                ADD     R2, R5, #0      @ args
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x33C12    @ cmd
                STR     R7, [R4]
                STR     R6, [R4,#4]
                BL      sdmmc_send_command
                ADD     R0, R4, #0
                BL      sub_80A3A4C
                POP     {R3-R7,PC}
@ End of function sdmmc_sdcard_readsectors

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global sdmmc_sdcard_writesector

sdmmc_sdcard_writesector:
                PUSH    {R4-R6,LR}
                LDR     R4, =dword_80A6F58
                ADD     R5, R0, #0
                LDR     R3, [R4,#0x20]
                ADD     R6, R1, #0
                CMP     R3, #0
                BNE     sdmmc_sdcard_writesector_
                LSL     R5, R0, #9 @SDMC works with raw addresses not sectors

loc_80A3C8E_:                            @ CODE XREF: sdmmc_sdcard_writesectors+Ej
                LDR     R3, [R4,#0x2C]
                CMP     R3, #0
                BEQ     sdmmc_sdcard_writesector_

loc_80A3C94_:                            @ CODE XREF: sdmmc_sdcard_writesectors:loc_80A3C94j
                B       loc_80A3C94_
				
sdmmc_sdcard_writesector_:
                BL      sub_80A38D4
                MOV     R3, #0x80
                LSL     R3, R3, #2 @ 0x200
                ADD     R2, R5, #0      @ args
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x50C18    @ cmd
                STR     R6, [R4]
                STR     R3, [R4,#4]
                BL      sdmmc_send_command
                ADD     R0, R4, #0
                BL      sub_80A3A4C
                POP     {R4-R6,PC}
@ End of function sdmmc_sdcard_readsectors

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global sdmmc_sdcard_writesectors

sdmmc_sdcard_writesectors:                     @ CODE XREF: menu_update+77Cp
                                        @ sub_80A3E9A+1Cp
                PUSH    {R3-R7,LR}
                LDR     R4, =dword_80A6F58
                ADD     R5, R0, #0
                LDR     R3, [R4,#0x20]
                ADD     R7, R2, #0
                ADD     R6, R1, #0
                CMP     R3, #0
                BNE     loc_80A3C8E
                LSL     R5, R0, #9

loc_80A3C8E:                            @ CODE XREF: sdmmc_sdcard_writesectors+Ej
                LDR     R3, [R4,#0x2C]
                CMP     R3, #0
                BEQ     loc_80A3C96

loc_80A3C94:                            @ CODE XREF: sdmmc_sdcard_writesectors:loc_80A3C94j
                B       loc_80A3C94
@ ---------------------------------------------------------------------------

loc_80A3C96:                            @ CODE XREF: sdmmc_sdcard_writesectors+16j
                ADD     R0, R4, #0
                BL      sub_80A38D4
                LDR     R3, =0x10006008
                MOV     R2, #0x80
                LSL     R2, R2, #1
                STRH    R2, [R3]
                LSL     R3, R6, #0x10
                LDR     R2, =0x1000600A
                LSR     R3, R3, #0x10
                STRH    R3, [R2]
                LSL     R6, R6, #9
                ADD     R2, R5, #0      @ args
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x52C19    @ cmd
                STR     R7, [R4]
                STR     R6, [R4,#4]
                BL      sdmmc_send_command
                ADD     R0, R4, #0
                BL      sub_80A3A4C
                POP     {R3-R7,PC}
@ End of function sdmmc_sdcard_writesectors

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global Nand_Init

Nand_Init:                              @ CODE XREF: sub_80A244C+12p
                PUSH    {R4-R6,LR}
                LDR     R4, =dword_80A6F20
                ADD     R0, R4, #0
                BL      sub_80A38D4
                MOV     R0, #0xF0
                LSL     R0, R0, #8
                BL      sub_80A3910
                MOV     R1, #0          @ cmd
                ADD     R0, R4, #0      @ ctx
                ADD     R2, R1, #0      @ args
                BL      sdmmc_send_command

loc_80A3CF0:                            @ CODE XREF: Nand_Init+30j
                                        @ Nand_Init+36j
                LDR     R4, =dword_80A6F20
                MOV     R2, #0x80
                LDR     R1, =0x10701    @ cmd
                ADD     R0, R4, #0      @ ctx
                LSL     R2, R2, #0xD
                BL      sdmmc_send_command
                LDR     R1, [R4,#8]
                MOV     R6, #1
                TST     R1, R6
                BEQ     loc_80A3CF0
                LDR     R2, [R4,#0xC]
                CMP     R2, #0
                BGE     loc_80A3CF0
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x10602    @ cmd
                MOV     R2, #0          @ args
                BL      sdmmc_send_command
                LDR     R3, [R4,#8]
                MOV     R5, #4
                TST     R3, R5
                BEQ     loc_80A3D24

loc_80A3D1E:                            @ CODE XREF: Nand_Init+60j
                                        @ Nand_Init+72j ...
                MOV     R0, #1
                NEG     R0, R0
                B       locret_80A3DC6
@ ---------------------------------------------------------------------------

loc_80A3D24:                            @ CODE XREF: Nand_Init+48j
                LDRH    R2, [R4,#0x1C]
                LDR     R1, =0x10403    @ cmd
                LSL     R2, R2, #0x10
                ADD     R0, R4, #0      @ ctx
                BL      sdmmc_send_command
                LDR     R1, [R4,#8]
                TST     R1, R5
                BNE     loc_80A3D1E
                LDRH    R2, [R4,#0x1C]
                LDR     R1, =0x10609    @ cmd
                LSL     R2, R2, #0x10
                ADD     R0, R4, #0      @ ctx
                BL      sdmmc_send_command
                LDR     R1, [R4,#8]
                AND     R1, R5
                BNE     loc_80A3D1E
                ADD     R0, R4, #0
                ADD     R0, #0xC
                BL      sub_80A3A54
                STR     R0, [R4,#0x30]
                ADD     R0, R6, #0
                STR     R6, [R4,#0x24]
                BL      sub_80A38A0
                LDRH    R2, [R4,#0x1C]
                ADD     R0, R4, #0      @ ctx
                LSL     R2, R2, #0x10
                LDR     R1, =0x10407    @ cmd
                BL      sdmmc_send_command
                LDR     R2, [R4,#8]
                TST     R2, R5
                BNE     loc_80A3D1E
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x10506    @ cmd
                LDR     R2, =0x3B70100  @ args
                STR     R6, [R4,#0x28]
                BL      sdmmc_send_command
                LDR     R3, [R4,#8]
                TST     R3, R5
                BNE     loc_80A3D1E
                LDR     R1, =0x10506    @ cmd
                ADD     R0, R4, #0      @ ctx
                LDR     R2, =0x3B90100  @ args
                BL      sdmmc_send_command
                LDR     R1, [R4,#8]
                TST     R1, R5
                BNE     loc_80A3D1E
                LDRH    R2, [R4,#0x1C]
                ADD     R0, R4, #0      @ ctx
                LSL     R2, R2, #0x10
                LDR     R1, =0x1040D    @ cmd
                BL      sdmmc_send_command
                LDR     R2, [R4,#8]
                TST     R2, R5
                BNE     loc_80A3D1E
                MOV     R2, #0x80
                ADD     R0, R4, #0      @ ctx
                LDR     R1, =0x10410    @ cmd
                LSL     R2, R2, #2
                BL      sdmmc_send_command
                LDR     R3, [R4,#8]
                TST     R3, R5
                BNE     loc_80A3D1E
                LDR     R3, =dword_80A6F20
                MOV     R2, #0x80
                LDR     R1, [R3,#0x24]
                LSL     R2, R2, #2
                ORR     R2, R1
                LDR     R0, =dword_80A6F58
                STR     R2, [R3,#0x24]
                BL      sub_80A38D4
                MOV     R0, #0

locret_80A3DC6:                         @ CODE XREF: Nand_Init+4Ej
                POP     {R4-R6,PC}
@ End of function Nand_Init

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global sdmmc_nand_readsectors

sdmmc_nand_readsectors:                     @ CODE XREF: menu_update+770p
                PUSH    {R3-R7,LR}
                LDR     R4, =dword_80A6F20
                ADD     R5, R0, #0
                LDR     R3, [R4,#0x20]
                ADD     R7, R2, #0
                ADD     R6, R1, #0
                CMP     R3, #0
                BNE     loc_80A3E0A
                LSL     R5, R0, #9

loc_80A3E0A:                            @ CODE XREF: sdmmc_nand_readsectors+Ej
                ADD     R0, R4, #0
                BL      sub_80A38D4
                LDR     R3, =0x10006008
                MOV     R2, #0x80
                LSL     R2, R2, #1
                STRH    R2, [R3]
                LSL     R3, R6, #0x10
                LDR     R2, =0x1000600A
                LSR     R3, R3, #0x10
                STRH    R3, [R2]
                LDR     R1, =0x33C12    @ cmd
                ADD     R2, R5, #0      @ args
                LSL     R6, R6, #9
                ADD     R0, R4, #0      @ ctx
                STR     R7, [R4]
                STR     R6, [R4,#4]
                BL      sdmmc_send_command
                LDR     R0, =dword_80A6F58
                BL      sub_80A38D4
                ADD     R0, R4, #0
                BL      sub_80A3A4C
                POP     {R3-R7,PC}
@ End of function sdmmc_nand_readsectors

@ ---------------------------------------------------------------------------
.pool

sub_80A6754:                            @ CODE XREF: decrypt+36p
                                        @ decrypt+5Ap ...
                PUSH    {R4-R7,LR}
                MOV     R12, R3
                LSL     R7, R2, #0x10
                LSL     R3, R0, #0x10
                LSR     R3, R3, #0x10
                LSR     R7, R7, #0x10
                LSR     R5, R2, #0x10
                ADD     R4, R0, #0
                LSR     R0, R0, #0x10
                ADD     R6, R3, #0
                MUL     R6, R7
                MUL     R3, R5
                MUL     R7, R0
                MUL     R5, R0
                ADD     R3, R7, R3
                LSR     R0, R6, #0x10
                ADD     R0, R3, R0
                CMP     R7, R0
                BLS     loc_80A6780
                MOVS    R3, #0x80
                LSL     R3, R3, #9
                ADD     R5, R5, R3

loc_80A6780:                            @ CODE XREF: sub_80A6754+24j
                LSR     R3, R0, #0x10
                ADD     R5, R5, R3
                ADD     R3, R4, #0
                MOV     R4, R12
                MUL     R4, R3
                MUL     R2, R1
                LSL     R6, R6, #0x10
                LSR     R6, R6, #0x10
                ADD     R1, R4, R2
                LSL     R0, R0, #0x10
                ADD     R0, R0, R6
                ADD     R1, R1, R5
                POP     {R4-R7}
                POP     {R2}
                BX      R2
@ End of function sub_80A6754

@ ---------------------------------------------------------------------------
.pool

@ =============== S U B R O U T I N E =======================================

.global sdmmc_sdcard_init

sdmmc_sdcard_init:                     @ CODE XREF: menu_update+770p
				PUSH    {LR}
                BL      InitSD
                @BL      Nand_Init
                BL      SD_Init
				POP     {PC}

@ End of function sdmmc_readsectors2

@ ---------------------------------------------------------------------------

.align 4

dword_80A6F20:  .long 0                 @ DATA XREF: InitSD+2o
                                        @ ROM:off_80A39F0o ...
dword_80A6F24:  .long 0
                .long 0                 @ DATA XREF: Nand_Init+2Ar
                                        @ Nand_Init+42r ...
                .long 0                 @ DATA XREF: Nand_Init+32r
                .long 0
                .long 0
                .long 0
dword_80A6F3C:  .long 0                 @ DATA XREF: InitSD+12w
                                        @ Nand_Init:loc_80A3D24r ...
dword_80A6F40:  .long 0                 @ DATA XREF: InitSD+Cw
                                        @ sdmmc_readsectors2+6r
dword_80A6F44:  .long 0                 @ DATA XREF: InitSD+14w
                                        @ Nand_Init+80w ...
dword_80A6F48:  .long 0                 @ DATA XREF: InitSD+Ew
                                        @ Nand_Init+9Ew
dword_80A6F4C:  .long 0                 @ DATA XREF: InitSD+16w
dword_80A6F50:  .long 0                 @ DATA XREF: Nand_Init+7Cw
                                        @ sub_80A3E54+2o
dword_80A6F54:  .long 0                 @ DATA XREF: InitSD+10w
dword_80A6F58:  .long 0                 @ DATA XREF: InitSD+6o
                                        @ ROM:off_80A39F4o ...
                .long 0
                .long 0                 @ DATA XREF: SD_Init+28r
                                        @ SD_Init+4Ar
                .long 0                 @ DATA XREF: SD_Init+52r
                .long 0
                .long 0
                .long 0
dword_80A6F74:  .long 0                 @ DATA XREF: InitSD+1Cw
                                        @ SD_Init+34r
dword_80A6F78:  .long 0                 @ DATA XREF: InitSD+1Ew
                                        @ sdmmc_sdcard_readsectors+6r ...
dword_80A6F7C:  .long 0                 @ DATA XREF: InitSD+1Aw
dword_80A6F80:  .long 0                 @ DATA XREF: InitSD+20w
dword_80A6F84:  .long 0                 @ DATA XREF: InitSD+22w
dword_80A6F88:  .long 0                 @ DATA XREF: sub_80A3C20+2r
dword_80A6F8C:  .long 0                 @ DATA XREF: InitSD+24w
                .long 0
				.long 0
				.long 0
				.long 0
				.long 0
				.long 0
				.long 0
				.long 0
				.long 0
				

