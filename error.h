/*
    sticksc - pseudo p2p implementation of sticks/chopsticks game
    Copyright (C) 2025 nacl-gb3 (Gary Bond III)
    GitHub Link: https://github.com/nacl-gb3/sticksc/tree/main

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef STICKSC_ERROR
#define STICKSC_ERROR

#define STDIN_ERROR -1
#define THREAD_CREATE_ERROR -2
#define SERVER_CREATE_ERROR -3
#define CONNECTION_CREATE_ERROR -4
#define ENV_GET_ERROR -5
#define ARGS_ERROR -6
#define SERVER_SEND_ERROR -7
#define SERVER_RECV_ERROR -8
#define CONNECTION_SEND_ERROR -9
#define CONNECTION_RECV_ERROR -10
#define IO_ERROR -11
#define NOT_REACHABLE -254
#define NOT_IMPLEMENTED -255

#endif /* ifdef ndef STICKSC_ERROR */
