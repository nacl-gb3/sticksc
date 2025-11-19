/*
    sticksc - network implementation of sticks/chopsticks game
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

#ifndef STICKSC_SERVER
#define STICKSC_SERVER

#include <stdbool.h>
#include <stdint.h>

int server_init(uint16_t host_port, bool type);
int connection_init(uint16_t host_port);
char *connection_wait();
void *server_run(void *arg);
void server_stop();
void server_wait_to_stop();

int turn_await();
void turn_complete(int err);

#endif /* ifndef STICKSC_SERVER */
