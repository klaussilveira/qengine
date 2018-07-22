/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/**
 * @page playerframes Player Frame Table
 *
 * Use this reference table when creating animations
 * for new player models.
 *
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * |---------------|---------|--------|---------------------------|-------------|-------|-----|
 * | STAND         | 1       |        | idle                      | stand01     | 1     | 1   |
 * |               | 2       |        | idle                      | stand02     | 2     | 2   |
 * |               | 3       |        | idle                      | stand03     | 3     | 3   |
 * |               | 4       |        | idle                      | stand04     | 4     | 4   |
 * |               | 5       |        | idle                      | stand05     | 5     | 5   |
 * |               | 6       |        | idle                      | stand06     | 6     | 6   |
 * |               | 7       |        | idle                      | stand07     | 7     | 7   |
 * |               | 8       |        | idle                      | stand08     | 8     | 8   |
 * |               | 9       |        | idle                      | stand09     | 9     | 9   |
 * |               | 10      |        | idle                      | stand10     | 10    | 10  |
 * |               | 11      |        | idle                      | stand11     | 11    | 11  |
 * |               | 12      |        | idle                      | stand12     | 12    | 12  |
 * |               | 13      |        | idle                      | stand13     | 13    | 13  |
 * |               | 14      |        | idle                      | stand14     | 14    | 14  |
 * |               | 15      |        | idle                      | stand15     | 15    | 15  |
 * |               | 16      |        | idle                      | stand16     | 16    | 16  |
 * |               | 17      |        | idle                      | stand17     | 17    | 17  |
 * |               | 18      |        | idle                      | stand18     | 18    | 18  |
 * |               | 19      |        | idle                      | stand19     | 19    | 19  |
 * |               | 20      |        | idle                      | stand20     | 20    | 20  |
 * |               | 21      |        | idle                      | stand21     | 21    | 21  |
 * |               | 22      |        | idle                      | stand22     | 22    | 22  |
 * |               | 23      |        | idle                      | stand23     | 23    | 23  |
 * |               | 24      |        | idle                      | stand24     | 24    | 24  |
 * |               | 25      |        | idle                      | stand25     | 25    | 25  |
 * |               | 26      |        | idle                      | stand26     | 26    | 26  |
 * |               | 27      |        | idle                      | stand27     | 27    | 27  |
 * |               | 28      |        | idle                      | stand28     | 28    | 28  |
 * |               | 29      |        | idle                      | stand29     | 29    | 29  |
 * |               | 30      |        | idle                      | stand30     | 30    | 30  |
 * |               | 31      |        | idle                      | stand31     | 31    | 31  |
 * |               | 32      |        | idle                      | stand32     | 32    | 32  |
 * |               | 33      |        | idle                      | stand33     | 33    | 33  |
 * |               | 34      |        | idle                      | stand34     | 34    | 34  |
 * |               | 35      |        | idle                      | stand35     | 35    | 35  |
 * |               | 36      |        | idle                      | stand36     | 36    | 36  |
 * |               | 37      |        | idle                      | stand37     | 37    | 37  |
 * |               | 38      |        | idle                      | stand38     | 38    | 38  |
 * |               | 39      |        | idle                      | stand39     | 39    | 39  |
 * |               | 40      |        | default                   | stand40     | 40    | 40  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | RUN           | 1       |        | runs                      | run1        | 3     | 41  |
 * |               | 2       |        | plants foot               | run2        | 4     | 42  |
 * |               | 3       |        | runs                      | run3        | 5     | 43  |
 * |               | 4       |        | runs                      | run4        | 6     | 44  |
 * |               | 5       |        | plants foot               | run5        | 7     | 45  |
 * |               | 6       |        | runs                      | run6        | 8     | 46  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | ATTACK        | 1       |        | POW (-6.4, -10.3, 28.0)   | attack1     | 2     | 47  |
 * |               | 2       |        | recoils                   | attack2     | 3     | 48  |
 * |               | 3       |        | reaches up to cock weapon | attack3     | 4     | 49  |
 * |               | 4       |        | pulls slide back          | attack4     | 5     | 50  |
 * |               | 5       |        |  slide back               | attack5     | 6     | 51  |
 * |               | 6       |        | slide forward             | attack6     | 7     | 52  |
 * |               | 7       |        | returns to default        | attack7     | 8     | 53  |
 * |               | 8       |        | default                   | attack8     | 9     | 54  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | PAIN 1        | 1       |        | OUCH!                     | pain101     |       | 55  |
 * |               | 2       |        | recovers                  | pain102     |       | 56  |
 * |               | 3       |        | default                   | pain103     |       | 57  |
 * |               | 4       |        | default                   | pain104     |       | 58  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | PAIN 2        | 1       |        | OUCH!                     | pain201     |       | 59  |
 * |               | 2       |        | recovers                  | pain202     |       | 60  |
 * |               | 3       |        | default                   | pain203     |       | 61  |
 * |               | 4       |        | default                   | pain204     |       | 62  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | PAIN 3        | 1       |        | OUCH!                     | pain301     |       | 63  |
 * |               | 2       |        | recovers                  | pain302     |       | 64  |
 * |               | 3       |        | default                   | pain303     |       | 65  |
 * |               | 4       |        | default                   | pain304     |       | 66  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | JUMP          | 1       |        | JUMP                      | jump1       | 2     | 67  |
 * |               | 2       |        | airborn                   | jump2       | 3     | 68  |
 * |               | 3       |        | airborn                   | jump3       | 4     | 69  |
 * |               | 4       |        | LANDS                     | jump4       | 5     | 70  |
 * |               | 5       |        | recovers                  | jump5       | 6     | 71  |
 * |               | 6       |        | default                   | jump6       | 7     | 72  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | FLIPOFF       | 1       |        | reaches up for the finger | flip01      | 2     | 73  |
 * |               | 2       |        | reaches up for the finger | flip02      | 3     | 74  |
 * |               | 3       |        | fuck you                  | flip03      | 4     | 75  |
 * |               | 4       |        | fuck you                  | flip04      | 5     | 76  |
 * |               | 5       |        | fuck you                  | flip05      | 6     | 77  |
 * |               | 6       |        | fuck you                  | flip06      | 7     | 78  |
 * |               | 7       |        | fuck you                  | flip07      | 8     | 79  |
 * |               | 8       |        | fuck you                  | flip08      | 9     | 80  |
 * |               | 9       |        | fuck you                  | flip09      | 10    | 81  |
 * |               | 10      |        | returns to default        | flip10      | 11    | 82  |
 * |               | 11      |        | returns to default        | flip11      | 12    | 83  |
 * |               | 12      |        | default                   | flip12      | 13    | 84  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | SALUTE        | 1       |        | brings arm up             | salute01    | 2     | 85  |
 * |               | 2       |        | brings arm up             | salute02    | 3     | 86  |
 * |               | 3       |        | SALUTE!                   | salute03    | 4     | 87  |
 * |               | 4       |        | holds salute              | salute04    | 5     | 88  |
 * |               | 5       |        | holds salute              | salute05    | 6     | 89  |
 * |               | 6       |        | holds salute              | salute06    | 7     | 90  |
 * |               | 7       |        | pops salute               | salute07    | 8     | 91  |
 * |               | 8       |        | holds                     | salute08    | 9     | 92  |
 * |               | 9       |        | returns to default        | salute09    | 10    | 93  |
 * |               | 10      |        | returns to default        | salute10    | 11    | 94  |
 * |               | 11      |        | default                   | salute11    | 13    | 95  |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | TAUNT         | 1       |        | reaches for crotch        | taunt01     | 2     | 96  |
 * |               | 2       |        | reaches for crotch        | taunt02     | 3     | 97  |
 * |               | 3       |        | grabs crotch              | taunt03     | 4     | 98  |
 * |               | 4       |        | grabs crotch              | taunt04     | 5     | 99  |
 * |               | 5       |        | grabs crotch              | taunt05     | 6     | 100 |
 * |               | 6       |        | grabs crotch              | taunt06     | 7     | 101 |
 * |               | 7       |        | grabs crotch              | taunt07     | 8     | 102 |
 * |               | 8       |        | grabs UP!                 | taunt08     | 9     | 103 |
 * |               | 9       |        | grabs crotch              | taunt09     | 10    | 104 |
 * |               | 10      |        | grabs crotch              | taunt10     | 11    | 105 |
 * |               | 11      |        | grabs crotch              | taunt11     | 12    | 106 |
 * |               | 12      |        | grabs UP!                 | taunt12     | 13    | 107 |
 * |               | 13      |        | grabs crotch              | taunt13     | 14    | 108 |
 * |               | 14      |        | returns to default        | taunt14     | 15    | 109 |
 * |               | 15      |        | returns to default        | taunt15     | 16    | 110 |
 * |               | 16      |        | returns to default        | taunt16     | 17    | 111 |
 * |               | 17      |        | default                   | taunt17     | 18    | 112 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | WAVE          | 1       |        | brings arm up to wave     | wave01      | 2     | 113 |
 * |               | 2       |        | brings arm up to wave     | wave02      | 3     | 114 |
 * |               | 3       |        | waves                     | wave03      | 4     | 115 |
 * |               | 4       |        | waves                     | wave04      | 5     | 116 |
 * |               | 5       |        | waves                     | wave05      | 6     | 117 |
 * |               | 6       |        | waves                     | wave06      | 7     | 118 |
 * |               | 7       |        | returns to default        | wave07      | 8     | 119 |
 * |               | 8       |        | returns to default        | wave08      | 9     | 120 |
 * |               | 9       |        | returns to default        | wave09      | 10    | 121 |
 * |               | 10      |        | returns to default        | wave10      | 11    | 122 |
 * |               | 11      |        | default                   | wave11      | 12    | 123 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | POINT         | 1       |        | raises hand to point      | point01     | 2     | 124 |
 * |               | 2       |        | raises hand to point      | point02     | 3     | 125 |
 * |               | 3       |        | raises hand to point      | point03     | 4     | 126 |
 * |               | 4       |        | points                    | point04     | 5     | 127 |
 * |               | 5       |        | points                    | point05     | 6     | 128 |
 * |               | 6       |        | points                    | point06     | 7     | 129 |
 * |               | 7       |        | points                    | point07     | 8     | 130 |
 * |               | 8       |        | points                    | point08     | 9     | 131 |
 * |               | 9       |        | points                    | point09     | 10    | 132 |
 * |               | 10      |        | returns to default        | point10     | 11    | 133 |
 * |               | 11      |        | returns to default        | point11     | 12    | 134 |
 * |               | 12      |        | default                   | point12     | 13    | 135 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | CROUCH STAND  | 1       |        | idle                      | crstnd01    | 3     | 136 |
 * |               | 2       |        | idle                      | crstnd02    | 4     | 137 |
 * |               | 3       |        | idle                      | crstnd03    | 5     | 138 |
 * |               | 4       |        | idle                      | crstnd04    | 6     | 139 |
 * |               | 5       |        | idle                      | crstnd05    | 7     | 140 |
 * |               | 6       |        | idle                      | crstnd06    | 8     | 141 |
 * |               | 7       |        | idle                      | crstnd07    | 9     | 142 |
 * |               | 8       |        | idle                      | crstnd08    | 10    | 143 |
 * |               | 9       |        | idle                      | crstnd09    | 11    | 144 |
 * |               | 10      |        | idle                      | crstnd10    | 12    | 145 |
 * |               | 11      |        | idle                      | crstnd11    | 13    | 146 |
 * |               | 12      |        | idle                      | crstnd12    | 14    | 147 |
 * |               | 13      |        | idle                      | crstnd13    | 15    | 148 |
 * |               | 14      |        | idle                      | crstnd14    | 16    | 149 |
 * |               | 15      |        | idle                      | crstnd15    | 17    | 150 |
 * |               | 16      |        | idle                      | crstnd16    | 18    | 151 |
 * |               | 17      |        | idle                      | crstnd17    | 19    | 152 |
 * |               | 18      |        | idle                      | crstnd18    | 20    | 153 |
 * |               | 19      |        | idle                      | crstnd19    | 21    | 154 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | CROUCH WALK   | 1       |        | plants foot               | crwalk1     | 3     | 155 |
 * |               | 2       |        | walks                     | crwalk2     | 4     | 156 |
 * |               | 3       |        | walks                     | crwalk3     | 5     | 157 |
 * |               | 4       |        | plants foot               | crwalk4     | 6     | 158 |
 * |               | 5       |        | walks                     | crwalk5     | 7     | 159 |
 * |               | 6       |        | walks                     | crwalk6     | 8     | 160 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | CROUCH ATTACK | 1       |        | POW                       | crattak1    | 4     | 161 |
 * |               | 2       |        | recoils                   | crattak2    | 5     | 162 |
 * |               | 3       |        | recoils                   | crattak3    | 6     | 163 |
 * |               | 4       |        | reaches for slide         | crattak4    | 7     | 164 |
 * |               | 5       |        | pulls slide forward       | crattak5    | 8     | 165 |
 * |               | 6       |        | pulls slide back          | crattak6    | 9     | 166 |
 * |               | 7       |        |  returns to default       | crattak7    | 10    | 167 |
 * |               | 8       |        |  returns to default       | crattak8    | 11    | 168 |
 * |               | 9       |        | default                   | crattak9    | 12    | 169 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | CROUCH PAIN   | 1       | -2     | OUCH!                     | crpain1.tri | 3     | 170 |
 * |               | 2       | -2     | recoils                   | crpain2.tri | 4     | 171 |
 * |               | 3       | 1      | recoils                   | crpain3.tri | 5     | 172 |
 * |               | 4       | 3      | crouch default            | crpain4.tri | 6     | 173 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | CROUCH DEATH  | 1       |        | OUCH                      | crdeath1    | 3     | 174 |
 * |               | 2       |        | dying                     | crdeath2    | 4     | 175 |
 * |               | 3       |        | dying                     | crdeath3    | 5     | 176 |
 * |               | 4       |        | dying                     | crdeath4    | 6     | 177 |
 * |               | 5       |        | dead                      | crdeath5    | 7     | 178 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | DEATH 1       | 1       | 0      | OUCH                      | death101    |       | 179 |
 * |               | 2       | 0      | body settles              | death102    |       | 180 |
 * |               | 3       | -14    | body settles              | death103    |       | 181 |
 * |               | 4       | 19     | body settles              | death104    |       | 182 |
 * |               | 5       | 5      | body settles              | death105    |       | 183 |
 * |               | 6       | -2     | dead                      | death106    |       | 184 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | DEATH 2       | 1       | 0      | OUCH                      | death201    |       | 185 |
 * |               | 2       | 0      | body settles              | death202    |       | 186 |
 * |               | 3       | -11    | body settles              | death203    |       | 187 |
 * |               | 4       | -5     | body settles              | death204    |       | 188 |
 * |               | 5       | 0      | body settles              | death205    |       | 189 |
 * |               | 6       | 0      | dead                      | death206    |       | 190 |
 * | ANIMATION     | FRAME # | OFFSET | ACTION DESCRIPTION        | FILE NAME   | NOTES |     |
 * | DEATH 3       | 1       |        | OUCH                      | death301    |       | 191 |
 * |               | 2       |        | body settles              | death302    |       | 192 |
 * |               | 3       |        | body settles              | death303    |       | 193 |
 * |               | 4       |        | body settles              | death304    |       | 194 |
 * |               | 5       |        | body settles              | death305    |       | 195 |
 * |               | 6       |        | body settles              | death306    |       | 196 |
 * |               | 7       |        | body settles              | death307    |       | 197 |
 * |               | 8       |        | dead                      | death308    |       | 198 |
 */
#define FRAME_stand01 0
#define FRAME_stand02 1
#define FRAME_stand03 2
#define FRAME_stand04 3
#define FRAME_stand05 4
#define FRAME_stand06 5
#define FRAME_stand07 6
#define FRAME_stand08 7
#define FRAME_stand09 8
#define FRAME_stand10 9
#define FRAME_stand11 10
#define FRAME_stand12 11
#define FRAME_stand13 12
#define FRAME_stand14 13
#define FRAME_stand15 14
#define FRAME_stand16 15
#define FRAME_stand17 16
#define FRAME_stand18 17
#define FRAME_stand19 18
#define FRAME_stand20 19
#define FRAME_stand21 20
#define FRAME_stand22 21
#define FRAME_stand23 22
#define FRAME_stand24 23
#define FRAME_stand25 24
#define FRAME_stand26 25
#define FRAME_stand27 26
#define FRAME_stand28 27
#define FRAME_stand29 28
#define FRAME_stand30 29
#define FRAME_stand31 30
#define FRAME_stand32 31
#define FRAME_stand33 32
#define FRAME_stand34 33
#define FRAME_stand35 34
#define FRAME_stand36 35
#define FRAME_stand37 36
#define FRAME_stand38 37
#define FRAME_stand39 38
#define FRAME_stand40 39
#define FRAME_run1 40
#define FRAME_run2 41
#define FRAME_run3 42
#define FRAME_run4 43
#define FRAME_run5 44
#define FRAME_run6 45
#define FRAME_attack1 46
#define FRAME_attack2 47
#define FRAME_attack3 48
#define FRAME_attack4 49
#define FRAME_attack5 50
#define FRAME_attack6 51
#define FRAME_attack7 52
#define FRAME_attack8 53
#define FRAME_pain101 54
#define FRAME_pain102 55
#define FRAME_pain103 56
#define FRAME_pain104 57
#define FRAME_pain201 58
#define FRAME_pain202 59
#define FRAME_pain203 60
#define FRAME_pain204 61
#define FRAME_pain301 62
#define FRAME_pain302 63
#define FRAME_pain303 64
#define FRAME_pain304 65
#define FRAME_jump1 66
#define FRAME_jump2 67
#define FRAME_jump3 68
#define FRAME_jump4 69
#define FRAME_jump5 70
#define FRAME_jump6 71
#define FRAME_flip01 72
#define FRAME_flip02 73
#define FRAME_flip03 74
#define FRAME_flip04 75
#define FRAME_flip05 76
#define FRAME_flip06 77
#define FRAME_flip07 78
#define FRAME_flip08 79
#define FRAME_flip09 80
#define FRAME_flip10 81
#define FRAME_flip11 82
#define FRAME_flip12 83
#define FRAME_salute01 84
#define FRAME_salute02 85
#define FRAME_salute03 86
#define FRAME_salute04 87
#define FRAME_salute05 88
#define FRAME_salute06 89
#define FRAME_salute07 90
#define FRAME_salute08 91
#define FRAME_salute09 92
#define FRAME_salute10 93
#define FRAME_salute11 94
#define FRAME_taunt01 95
#define FRAME_taunt02 96
#define FRAME_taunt03 97
#define FRAME_taunt04 98
#define FRAME_taunt05 99
#define FRAME_taunt06 100
#define FRAME_taunt07 101
#define FRAME_taunt08 102
#define FRAME_taunt09 103
#define FRAME_taunt10 104
#define FRAME_taunt11 105
#define FRAME_taunt12 106
#define FRAME_taunt13 107
#define FRAME_taunt14 108
#define FRAME_taunt15 109
#define FRAME_taunt16 110
#define FRAME_taunt17 111
#define FRAME_wave01 112
#define FRAME_wave02 113
#define FRAME_wave03 114
#define FRAME_wave04 115
#define FRAME_wave05 116
#define FRAME_wave06 117
#define FRAME_wave07 118
#define FRAME_wave08 119
#define FRAME_wave09 120
#define FRAME_wave10 121
#define FRAME_wave11 122
#define FRAME_point01 123
#define FRAME_point02 124
#define FRAME_point03 125
#define FRAME_point04 126
#define FRAME_point05 127
#define FRAME_point06 128
#define FRAME_point07 129
#define FRAME_point08 130
#define FRAME_point09 131
#define FRAME_point10 132
#define FRAME_point11 133
#define FRAME_point12 134
#define FRAME_crstnd01 135
#define FRAME_crstnd02 136
#define FRAME_crstnd03 137
#define FRAME_crstnd04 138
#define FRAME_crstnd05 139
#define FRAME_crstnd06 140
#define FRAME_crstnd07 141
#define FRAME_crstnd08 142
#define FRAME_crstnd09 143
#define FRAME_crstnd10 144
#define FRAME_crstnd11 145
#define FRAME_crstnd12 146
#define FRAME_crstnd13 147
#define FRAME_crstnd14 148
#define FRAME_crstnd15 149
#define FRAME_crstnd16 150
#define FRAME_crstnd17 151
#define FRAME_crstnd18 152
#define FRAME_crstnd19 153
#define FRAME_crwalk1 154
#define FRAME_crwalk2 155
#define FRAME_crwalk3 156
#define FRAME_crwalk4 157
#define FRAME_crwalk5 158
#define FRAME_crwalk6 159
#define FRAME_crattak1 160
#define FRAME_crattak2 161
#define FRAME_crattak3 162
#define FRAME_crattak4 163
#define FRAME_crattak5 164
#define FRAME_crattak6 165
#define FRAME_crattak7 166
#define FRAME_crattak8 167
#define FRAME_crattak9 168
#define FRAME_crpain1 169
#define FRAME_crpain2 170
#define FRAME_crpain3 171
#define FRAME_crpain4 172
#define FRAME_crdeath1 173
#define FRAME_crdeath2 174
#define FRAME_crdeath3 175
#define FRAME_crdeath4 176
#define FRAME_crdeath5 177
#define FRAME_death101 178
#define FRAME_death102 179
#define FRAME_death103 180
#define FRAME_death104 181
#define FRAME_death105 182
#define FRAME_death106 183
#define FRAME_death201 184
#define FRAME_death202 185
#define FRAME_death203 186
#define FRAME_death204 187
#define FRAME_death205 188
#define FRAME_death206 189
#define FRAME_death301 190
#define FRAME_death302 191
#define FRAME_death303 192
#define FRAME_death304 193
#define FRAME_death305 194
#define FRAME_death306 195
#define FRAME_death307 196
#define FRAME_death308 197

#define MODEL_SCALE 1.000000
