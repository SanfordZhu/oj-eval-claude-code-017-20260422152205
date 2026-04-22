#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace std;

const int MAX_USERS = 200000;
const int MAX_TRAINS = 200000;
const int MAX_STATIONS = 100;
const int MAX_ORDERS = 200000;
const int MAX_DATE_DAYS = 92;

int date_to_int(int month, int day) {
    return (month - 6) * 31 + (day - 1);
}

struct User {
    char username[21];
    char password[31];
    char name[11];
    char mailAddr[31];
    int privilege;
    bool online;
    int next;
};

User users[MAX_USERS];
int user_head[MAX_USERS];
int user_count = 0;

unsigned int hash_string(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int find_user(const char *username) {
    unsigned int idx = hash_string(username) % MAX_USERS;
    for (int i = user_head[idx]; i != -1; i = users[i].next) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

bool add_user(const char *cur_username, const char *username, const char *password,
              const char *name, const char *mailAddr, int privilege) {
    if (find_user(username) != -1) return false;

    if (user_count > 0) {
        int cur_idx = find_user(cur_username);
        if (cur_idx == -1 || !users[cur_idx].online) return false;
        if (privilege >= users[cur_idx].privilege) return false;
    } else {
        privilege = 10;
    }

    unsigned int idx = hash_string(username) % MAX_USERS;
    int new_idx = user_count++;
    strcpy(users[new_idx].username, username);
    strcpy(users[new_idx].password, password);
    strcpy(users[new_idx].name, name);
    strcpy(users[new_idx].mailAddr, mailAddr);
    users[new_idx].privilege = privilege;
    users[new_idx].online = false;
    users[new_idx].next = user_head[idx];
    user_head[idx] = new_idx;
    return true;
}

bool login(const char *username, const char *password) {
    int idx = find_user(username);
    if (idx == -1 || users[idx].online) return false;
    if (strcmp(users[idx].password, password) != 0) return false;
    users[idx].online = true;
    return true;
}

bool logout(const char *username) {
    int idx = find_user(username);
    if (idx == -1 || !users[idx].online) return false;
    users[idx].online = false;
    return true;
}

struct Train {
    char trainID[21];
    int stationNum;
    char stations[MAX_STATIONS][11];
    int seatNum;
    int prices[MAX_STATIONS];
    int startTime;
    int travelTimes[MAX_STATIONS];
    int stopoverTimes[MAX_STATIONS];
    int saleDateStart;
    int saleDateEnd;
    char type;
    bool released;
    int *seats;
    int next;
};

Train trains[MAX_TRAINS];
int train_head[MAX_TRAINS];
int train_count = 0;

int find_train(const char *trainID) {
    unsigned int idx = hash_string(trainID) % MAX_TRAINS;
    for (int i = train_head[idx]; i != -1; i = trains[i].next) {
        if (strcmp(trains[i].trainID, trainID) == 0) {
            return i;
        }
    }
    return -1;
}

bool add_train(const char *trainID, int stationNum, int seatNum,
               const char *stations_str, const char *prices_str,
               const char *startTime_str, const char *travelTimes_str,
               const char *stopoverTimes_str, const char *saleDate_str, char type) {
    if (find_train(trainID) != -1) return false;

    unsigned int idx = hash_string(trainID) % MAX_TRAINS;
    int new_idx = train_count++;
    strcpy(trains[new_idx].trainID, trainID);
    trains[new_idx].stationNum = stationNum;
    trains[new_idx].seatNum = seatNum;
    trains[new_idx].type = type;
    trains[new_idx].released = false;

    char temp[5000];
    strcpy(temp, stations_str);
    char *token = strtok(temp, "|");
    for (int i = 0; i < stationNum && token != NULL; i++) {
        strcpy(trains[new_idx].stations[i], token);
        token = strtok(NULL, "|");
    }

    strcpy(temp, prices_str);
    token = strtok(temp, "|");
    trains[new_idx].prices[0] = 0;
    for (int i = 1; i < stationNum && token != NULL; i++) {
        trains[new_idx].prices[i] = trains[new_idx].prices[i-1] + atoi(token);
        token = strtok(NULL, "|");
    }

    int hour, minute;
    sscanf(startTime_str, "%d:%d", &hour, &minute);
    trains[new_idx].startTime = hour * 60 + minute;

    strcpy(temp, travelTimes_str);
    token = strtok(temp, "|");
    for (int i = 0; i < stationNum - 1 && token != NULL; i++) {
        trains[new_idx].travelTimes[i] = atoi(token);
        token = strtok(NULL, "|");
    }

    if (stationNum > 2) {
        strcpy(temp, stopoverTimes_str);
        token = strtok(temp, "|");
        for (int i = 0; i < stationNum - 2 && token != NULL; i++) {
            trains[new_idx].stopoverTimes[i] = atoi(token);
            token = strtok(NULL, "|");
        }
    }

    int sm, sd, em, ed;
    sscanf(saleDate_str, "%d-%d|%d-%d", &sm, &sd, &em, &ed);
    trains[new_idx].saleDateStart = date_to_int(sm, sd);
    trains[new_idx].saleDateEnd = date_to_int(em, ed);

    trains[new_idx].seats = NULL;
    trains[new_idx].next = train_head[idx];
    train_head[idx] = new_idx;
    return true;
}

bool release_train(const char *trainID) {
    int idx = find_train(trainID);
    if (idx == -1 || trains[idx].released) return false;

    int days = trains[idx].saleDateEnd - trains[idx].saleDateStart + 1;
    int segs = trains[idx].stationNum - 1;
    trains[idx].seats = new int[days * segs];
    for (int i = 0; i < days * segs; i++) {
        trains[idx].seats[i] = trains[idx].seatNum;
    }

    trains[idx].released = true;
    return true;
}

bool delete_train(const char *trainID) {
    int idx = find_train(trainID);
    if (idx == -1 || trains[idx].released) return false;

    unsigned int hash_idx = hash_string(trainID) % MAX_TRAINS;
    int *p = &train_head[hash_idx];
    while (*p != idx) {
        p = &trains[*p].next;
    }
    *p = trains[idx].next;

    return true;
}

struct Order {
    char username[21];
    char trainID[21];
    char fromStation[11];
    char toStation[11];
    int date;
    int depTime;
    int arrTime;
    int price;
    int num;
    int status;
    int next;
    int prev;
};

Order orders[MAX_ORDERS];
int order_head[MAX_USERS];
int order_tail[MAX_USERS];
int order_count = 0;

void add_order(const char *username, const char *trainID, const char *from,
               const char *to, int date, int depTime, int arrTime, int price, int num, int status) {
    unsigned int hash_idx = hash_string(username) % MAX_USERS;

    int new_idx = order_count++;
    strcpy(orders[new_idx].username, username);
    strcpy(orders[new_idx].trainID, trainID);
    strcpy(orders[new_idx].fromStation, from);
    strcpy(orders[new_idx].toStation, to);
    orders[new_idx].date = date;
    orders[new_idx].depTime = depTime;
    orders[new_idx].arrTime = arrTime;
    orders[new_idx].price = price;
    orders[new_idx].num = num;
    orders[new_idx].status = status;
    orders[new_idx].next = -1;
    orders[new_idx].prev = order_tail[hash_idx];

    if (order_tail[hash_idx] != -1) {
        orders[order_tail[hash_idx]].next = new_idx;
    }
    if (order_head[hash_idx] == -1) {
        order_head[hash_idx] = new_idx;
    }
    order_tail[hash_idx] = new_idx;
}

struct PendingOrder {
    char username[21];
    char trainID[21];
    char fromStation[11];
    char toStation[11];
    int date;
    int num;
    int next;
};

PendingOrder pending_orders[MAX_ORDERS];
int pending_head[MAX_TRAINS];
int pending_count = 0;

void add_pending(const char *username, const char *trainID, const char *from,
                 const char *to, int date, int num) {
    unsigned int hash_idx = hash_string(trainID) % MAX_TRAINS;

    int new_idx = pending_count++;
    strcpy(pending_orders[new_idx].username, username);
    strcpy(pending_orders[new_idx].trainID, trainID);
    strcpy(pending_orders[new_idx].fromStation, from);
    strcpy(pending_orders[new_idx].toStation, to);
    pending_orders[new_idx].date = date;
    pending_orders[new_idx].num = num;
    pending_orders[new_idx].next = pending_head[hash_idx];
    pending_head[hash_idx] = new_idx;
}

int get_seat_available(Train &train, int dateIdx, int fromIdx, int toIdx) {
    int minSeats = train.seatNum;
    int base = dateIdx * (train.stationNum - 1);
    for (int i = fromIdx; i < toIdx; i++) {
        if (train.seats[base + i] < minSeats) {
            minSeats = train.seats[base + i];
        }
    }
    return minSeats;
}

void update_seats(Train &train, int dateIdx, int fromIdx, int toIdx, int delta) {
    int base = dateIdx * (train.stationNum - 1);
    for (int i = fromIdx; i < toIdx; i++) {
        train.seats[base + i] += delta;
    }
}

int find_station(Train &train, const char *station) {
    for (int i = 0; i < train.stationNum; i++) {
        if (strcmp(train.stations[i], station) == 0) {
            return i;
        }
    }
    return -1;
}

void process_pending(const char *trainID) {
    int train_idx = find_train(trainID);
    if (train_idx == -1) return;

    unsigned int hash_idx = hash_string(trainID) % MAX_TRAINS;
    bool changed = true;

    while (changed) {
        changed = false;
        int *p = &pending_head[hash_idx];
        while (*p != -1) {
            int cur = *p;
            Train &train = trains[train_idx];
            int fromIdx = find_station(train, pending_orders[cur].fromStation);
            int toIdx = find_station(train, pending_orders[cur].toStation);
            int dateIdx = pending_orders[cur].date - train.saleDateStart;

            if (fromIdx != -1 && toIdx != -1 && dateIdx >= 0 &&
                get_seat_available(train, dateIdx, fromIdx, toIdx) >= pending_orders[cur].num) {

                update_seats(train, dateIdx, fromIdx, toIdx, -pending_orders[cur].num);

                int price = train.prices[toIdx] - train.prices[fromIdx];
                int depMin = train.startTime;
                for (int i = 0; i < fromIdx; i++) {
                    depMin += train.travelTimes[i];
                    if (i < fromIdx - 1) depMin += train.stopoverTimes[i];
                }
                int arrMin = depMin;
                for (int i = fromIdx; i < toIdx; i++) {
                    arrMin += train.travelTimes[i];
                    if (i < toIdx - 1) arrMin += train.stopoverTimes[i];
                }

                add_order(pending_orders[cur].username, trainID,
                         pending_orders[cur].fromStation, pending_orders[cur].toStation,
                         pending_orders[cur].date, depMin, arrMin, price,
                         pending_orders[cur].num, 0);

                *p = pending_orders[cur].next;
                changed = true;
            } else {
                p = &pending_orders[cur].next;
            }
        }
    }
}

void query_train_output(const char *trainID, int date) {
    int idx = find_train(trainID);
    if (idx == -1) {
        printf("-1\n");
        return;
    }
    Train &train = trains[idx];

    int dateIdx = date - train.saleDateStart;
    if (dateIdx < 0 || dateIdx > train.saleDateEnd - train.saleDateStart) {
        printf("-1\n");
        return;
    }

    printf("%s %c\n", train.trainID, train.type);

    int curTime = train.startTime;
    for (int i = 0; i < train.stationNum; i++) {
        if (i == 0) {
            printf("%s xx-xx xx:xx -> ", train.stations[i]);
        } else {
            int arrTime = curTime;
            int arrDay = date + arrTime / (24 * 60);
            int arrHour = (arrTime / 60) % 24;
            int arrMinute = arrTime % 60;

            printf("%s %02d-%02d %02d:%02d -> ", train.stations[i], 6 + arrDay/31, arrDay%31+1, arrHour, arrMinute);

            if (i < train.stationNum - 1) {
                curTime += train.stopoverTimes[i-1];
            }
        }

        if (i < train.stationNum - 1) {
            int leaveDay = date + curTime / (24 * 60);
            int leaveHour = (curTime / 60) % 24;
            int leaveMinute = curTime % 60;
            printf("%02d-%02d %02d:%02d ", 6 + leaveDay/31, leaveDay%31+1, leaveHour, leaveMinute);
        } else {
            printf("xx-xx xx:xx ");
        }

        printf("%d ", train.prices[i]);

        if (i < train.stationNum - 1) {
            if (train.released) {
                printf("%d\n", get_seat_available(train, dateIdx, i, i+1));
            } else {
                printf("%d\n", train.seatNum);
            }
        } else {
            printf("x\n");
        }

        if (i < train.stationNum - 1) {
            curTime += train.travelTimes[i];
        }
    }
}

struct TicketResult {
    char trainID[21];
    char fromStation[11];
    char toStation[11];
    int depDay;
    int depTime;
    int arrTime;
    int price;
    int seat;
    int duration;
    int cost;
};

TicketResult ticket_results[MAX_TRAINS];
int ticket_result_count = 0;

void query_ticket(const char *from, const char *to, int date, const char *priority) {
    ticket_result_count = 0;

    for (int i = 0; i < train_count; i++) {
        if (!trains[i].released) continue;

        int fromIdx = find_station(trains[i], from);
        int toIdx = find_station(trains[i], to);

        if (fromIdx != -1 && toIdx != -1 && fromIdx < toIdx) {
            int depDay = date;
            int depMin = trains[i].startTime;
            for (int j = 0; j < fromIdx; j++) {
                depMin += trains[i].travelTimes[j];
                if (j < fromIdx - 1) depMin += trains[i].stopoverTimes[j];
            }

            int actualDepDay = depDay + depMin / (24 * 60);
            int dateIdx = actualDepDay - trains[i].saleDateStart;

            if (dateIdx >= 0 && dateIdx <= trains[i].saleDateEnd - trains[i].saleDateStart) {
                int arrMin = depMin;
                for (int j = fromIdx; j < toIdx; j++) {
                    arrMin += trains[i].travelTimes[j];
                    if (j < toIdx - 1) arrMin += trains[i].stopoverTimes[j];
                }

                int price = trains[i].prices[toIdx] - trains[i].prices[fromIdx];
                int seat = get_seat_available(trains[i], dateIdx, fromIdx, toIdx);

                strcpy(ticket_results[ticket_result_count].trainID, trains[i].trainID);
                strcpy(ticket_results[ticket_result_count].fromStation, from);
                strcpy(ticket_results[ticket_result_count].toStation, to);
                ticket_results[ticket_result_count].depDay = actualDepDay;
                ticket_results[ticket_result_count].depTime = depMin;
                ticket_results[ticket_result_count].arrTime = arrMin;
                ticket_results[ticket_result_count].price = price;
                ticket_results[ticket_result_count].seat = seat;
                ticket_results[ticket_result_count].duration = arrMin - depMin;
                ticket_results[ticket_result_count].cost = price;
                ticket_result_count++;
            }
        }
    }

    for (int i = 0; i < ticket_result_count - 1; i++) {
        for (int j = i + 1; j < ticket_result_count; j++) {
            bool swap = false;
            if (strcmp(priority, "time") == 0) {
                if (ticket_results[j].duration < ticket_results[i].duration ||
                    (ticket_results[j].duration == ticket_results[i].duration &&
                     strcmp(ticket_results[j].trainID, ticket_results[i].trainID) < 0)) {
                    swap = true;
                }
            } else {
                if (ticket_results[j].cost < ticket_results[i].cost ||
                    (ticket_results[j].cost == ticket_results[i].cost &&
                     strcmp(ticket_results[j].trainID, ticket_results[i].trainID) < 0)) {
                    swap = true;
                }
            }
            if (swap) {
                TicketResult temp = ticket_results[i];
                ticket_results[i] = ticket_results[j];
                ticket_results[j] = temp;
            }
        }
    }

    printf("%d\n", ticket_result_count);
    for (int i = 0; i < ticket_result_count; i++) {
        int depDay = ticket_results[i].depDay;
        int depHour = (ticket_results[i].depTime / 60) % 24;
        int depMinute = ticket_results[i].depTime % 60;
        int arrDay = depDay + ticket_results[i].arrTime / (24 * 60);
        int arrHour = (ticket_results[i].arrTime / 60) % 24;
        int arrMinute = ticket_results[i].arrTime % 60;

        printf("%s %s %02d-%02d %02d:%02d -> %s %02d-%02d %02d:%02d %d %d\n",
               ticket_results[i].trainID, ticket_results[i].fromStation,
               6 + depDay/31, depDay%31+1, depHour, depMinute,
               ticket_results[i].toStation,
               6 + arrDay/31, arrDay%31+1, arrHour, arrMinute,
               ticket_results[i].price, ticket_results[i].seat);
    }
}

void query_transfer(const char *from, const char *to, int date, const char *priority) {
    TicketResult best1, best2;
    bool found = false;
    int bestDuration = 2000000000;
    int bestCost = 2000000000;

    for (int i = 0; i < train_count; i++) {
        if (!trains[i].released) continue;

        int fromIdx = find_station(trains[i], from);
        if (fromIdx == -1) continue;

        int depDay = date;
        int depMin = trains[i].startTime;
        for (int j = 0; j < fromIdx; j++) {
            depMin += trains[i].travelTimes[j];
            if (j < fromIdx - 1) depMin += trains[i].stopoverTimes[j];
        }

        int actualDepDay = depDay + depMin / (24 * 60);
        int dateIdx1 = actualDepDay - trains[i].saleDateStart;

        if (dateIdx1 < 0 || dateIdx1 > trains[i].saleDateEnd - trains[i].saleDateStart) continue;

        for (int midIdx = fromIdx + 1; midIdx < trains[i].stationNum; midIdx++) {
            const char *midStation = trains[i].stations[midIdx];

            int arrMin = depMin;
            for (int j = fromIdx; j < midIdx; j++) {
                arrMin += trains[i].travelTimes[j];
                if (j < midIdx - 1) arrMin += trains[i].stopoverTimes[j];
            }

            int arrDay = actualDepDay + arrMin / (24 * 60);
            int transferDay = arrDay;

            for (int j = 0; j < train_count; j++) {
                if (i == j || !trains[j].released) continue;

                int midIdx2 = find_station(trains[j], midStation);
                int toIdx2 = find_station(trains[j], to);

                if (midIdx2 == -1 || toIdx2 == -1 || midIdx2 >= toIdx2) continue;

                int depDay2 = transferDay;
                int depMin2 = trains[j].startTime;
                for (int k = 0; k < midIdx2; k++) {
                    depMin2 += trains[j].travelTimes[k];
                    if (k < midIdx2 - 1) depMin2 += trains[j].stopoverTimes[k];
                }

                int actualDepDay2 = depDay2 + depMin2 / (24 * 60);
                int dateIdx2 = actualDepDay2 - trains[j].saleDateStart;

                if (dateIdx2 < 0 || dateIdx2 > trains[j].saleDateEnd - trains[j].saleDateStart) continue;

                int arrMin2 = depMin2;
                for (int k = midIdx2; k < toIdx2; k++) {
                    arrMin2 += trains[j].travelTimes[k];
                    if (k < toIdx2 - 1) arrMin2 += trains[j].stopoverTimes[k];
                }

                int arrDay2 = actualDepDay2 + arrMin2 / (24 * 60);

                int totalDuration = (arrDay2 * 24 * 60 + arrMin2) - (date * 24 * 60);
                int totalCost = (trains[i].prices[midIdx] - trains[i].prices[fromIdx]) +
                               (trains[j].prices[toIdx2] - trains[j].prices[midIdx2]);

                bool better = false;
                if (strcmp(priority, "time") == 0) {
                    if (!found || totalDuration < bestDuration ||
                        (totalDuration == bestDuration &&
                         (trains[i].prices[midIdx] - trains[i].prices[fromIdx]) <
                         (best1.arrTime - best1.depTime))) {
                        better = true;
                    }
                } else {
                    if (!found || totalCost < bestCost) {
                        better = true;
                    }
                }

                if (better) {
                    found = true;
                    bestDuration = totalDuration;
                    bestCost = totalCost;

                    strcpy(best1.trainID, trains[i].trainID);
                    strcpy(best1.fromStation, from);
                    strcpy(best1.toStation, midStation);
                    best1.depDay = actualDepDay;
                    best1.depTime = depMin;
                    best1.arrTime = arrMin;
                    best1.price = trains[i].prices[midIdx] - trains[i].prices[fromIdx];
                    best1.duration = arrMin - depMin;
                    best1.cost = best1.price;

                    strcpy(best2.trainID, trains[j].trainID);
                    strcpy(best2.fromStation, midStation);
                    strcpy(best2.toStation, to);
                    best2.depDay = actualDepDay2;
                    best2.depTime = depMin2;
                    best2.arrTime = arrMin2;
                    best2.price = trains[j].prices[toIdx2] - trains[j].prices[midIdx2];
                    best2.duration = arrMin2 - depMin2;
                    best2.cost = best2.price;
                }
            }
        }
    }

    if (!found) {
        printf("0\n");
        return;
    }

    for (int i = 0; i < 2; i++) {
        TicketResult &res = (i == 0) ? best1 : best2;
        int depDay = res.depDay;
        int depHour = (res.depTime / 60) % 24;
        int depMinute = res.depTime % 60;
        int arrDay = depDay + res.arrTime / (24 * 60);
        int arrHour = (res.arrTime / 60) % 24;
        int arrMinute = res.arrTime % 60;

        printf("%s %s %02d-%02d %02d:%02d -> %s %02d-%02d %02d:%02d %d %d\n",
               res.trainID, res.fromStation,
               6 + depDay/31, depDay%31+1, depHour, depMinute,
               res.toStation,
               6 + arrDay/31, arrDay%31+1, arrHour, arrMinute,
               res.price, 999999);
    }
}

int buy_ticket(const char *username, const char *trainID, int date, int num,
               const char *from, const char *to, bool queue) {
    int user_idx = find_user(username);
    if (user_idx == -1 || !users[user_idx].online) return -1;

    int train_idx = find_train(trainID);
    if (train_idx == -1 || !trains[train_idx].released) return -1;

    Train &train = trains[train_idx];
    int fromIdx = find_station(train, from);
    int toIdx = find_station(train, to);

    if (fromIdx == -1 || toIdx == -1 || fromIdx >= toIdx) return -1;
    if (num <= 0 || num > train.seatNum) return -1;

    int depDay = date;
    int depMin = train.startTime;
    for (int j = 0; j < fromIdx; j++) {
        depMin += train.travelTimes[j];
        if (j < fromIdx - 1) depMin += train.stopoverTimes[j];
    }

    int actualDepDay = depDay + depMin / (24 * 60);
    int dateIdx = actualDepDay - train.saleDateStart;

    if (dateIdx < 0 || dateIdx > train.saleDateEnd - train.saleDateStart) return -1;

    int available = get_seat_available(train, dateIdx, fromIdx, toIdx);

    if (available >= num) {
        update_seats(train, dateIdx, fromIdx, toIdx, -num);

        int arrMin = depMin;
        for (int j = fromIdx; j < toIdx; j++) {
            arrMin += train.travelTimes[j];
            if (j < toIdx - 1) arrMin += train.stopoverTimes[j];
        }

        int price = train.prices[toIdx] - train.prices[fromIdx];

        add_order(username, trainID, from, to, actualDepDay, depMin, arrMin, price, num, 0);

        process_pending(trainID);

        return price * num;
    } else if (queue) {
        add_pending(username, trainID, from, to, actualDepDay, num);
        add_order(username, trainID, from, to, actualDepDay, depMin, depMin, 0, num, 1);
        return -2;
    }

    return -1;
}

void query_order(const char *username) {
    int user_idx = find_user(username);
    if (user_idx == -1 || !users[user_idx].online) {
        printf("-1\n");
        return;
    }

    unsigned int hash_idx = hash_string(username) % MAX_USERS;

    int count = 0;
    for (int i = order_tail[hash_idx]; i != -1; i = orders[i].prev) {
        count++;
    }

    printf("%d\n", count);

    for (int i = order_tail[hash_idx]; i != -1; i = orders[i].prev) {
        Order &ord = orders[i];
        const char *status_str;
        if (ord.status == 0) status_str = "success";
        else if (ord.status == 1) status_str = "pending";
        else status_str = "refunded";

        int depDay = ord.date;
        int depHour = (ord.depTime / 60) % 24;
        int depMinute = ord.depTime % 60;
        int arrDay = depDay + ord.arrTime / (24 * 60);
        int arrHour = (ord.arrTime / 60) % 24;
        int arrMinute = ord.arrTime % 60;

        printf("[%s] %s %s %02d-%02d %02d:%02d -> %s %02d-%02d %02d:%02d %d %d\n",
               status_str, ord.trainID, ord.fromStation,
               6 + depDay/31, depDay%31+1, depHour, depMinute,
               ord.toStation,
               6 + arrDay/31, arrDay%31+1, arrHour, arrMinute,
               ord.price, ord.num);
    }
}

int refund_ticket(const char *username, int n) {
    int user_idx = find_user(username);
    if (user_idx == -1 || !users[user_idx].online) return -1;

    unsigned int hash_idx = hash_string(username) % MAX_USERS;

    int count = 0;
    int target_idx = -1;
    for (int i = order_tail[hash_idx]; i != -1; i = orders[i].prev) {
        count++;
        if (count == n) {
            target_idx = i;
            break;
        }
    }

    if (n < 1 || target_idx == -1) return -1;

    Order &ord = orders[target_idx];

    if (ord.status == 2) return -1;

    if (ord.status == 0) {
        int train_idx = find_train(ord.trainID);
        if (train_idx != -1) {
            Train &train = trains[train_idx];
            int fromIdx = find_station(train, ord.fromStation);
            int toIdx = find_station(train, ord.toStation);
            int dateIdx = ord.date - train.saleDateStart;

            if (fromIdx != -1 && toIdx != -1 && dateIdx >= 0) {
                update_seats(train, dateIdx, fromIdx, toIdx, ord.num);
                process_pending(ord.trainID);
            }
        }
    }

    ord.status = 2;
    return 0;
}

void clean() {
    for (int i = 0; i < train_count; i++) {
        if (trains[i].seats != NULL) {
            delete[] trains[i].seats;
            trains[i].seats = NULL;
        }
    }

    user_count = 0;
    train_count = 0;
    order_count = 0;
    pending_count = 0;

    for (int i = 0; i < MAX_USERS; i++) {
        user_head[i] = -1;
        order_head[i] = -1;
        order_tail[i] = -1;
    }
    for (int i = 0; i < MAX_TRAINS; i++) {
        train_head[i] = -1;
        pending_head[i] = -1;
    }
}

char input[10000];
char cmd[100];
char args[20][100];

void parse_args(char *line) {
    int arg_count = 0;
    char *p = line;
    while (*p && arg_count < 20) {
        while (*p == ' ') p++;
        if (*p == '\0') break;
        if (*p == '-' && *(p+1)) {
            args[arg_count][0] = *(p+1);
            args[arg_count][1] = '\0';
            p += 2;
            while (*p == ' ') p++;
            char *val = args[arg_count] + 2;
            while (*p && *p != ' ' && *p != '\n') {
                *val++ = *p++;
            }
            *val = '\0';
            arg_count++;
        } else {
            p++;
        }
    }
}

const char* get_arg(char key) {
    for (int i = 0; i < 20; i++) {
        if (args[i][0] == key) {
            return args[i] + 2;
        }
    }
    return NULL;
}

bool has_arg(char key) {
    return get_arg(key) != NULL;
}

int main() {
    for (int i = 0; i < MAX_USERS; i++) {
        user_head[i] = -1;
        order_head[i] = -1;
        order_tail[i] = -1;
    }
    for (int i = 0; i < MAX_TRAINS; i++) {
        train_head[i] = -1;
        pending_head[i] = -1;
    }

    while (fgets(input, sizeof(input), stdin)) {
        char *p = strchr(input, '\n');
        if (p) *p = '\0';

        if (input[0] == '\0') continue;

        char *space = strchr(input, ' ');
        if (space) {
            *space = '\0';
            strcpy(cmd, input);
            parse_args(space + 1);
        } else {
            strcpy(cmd, input);
            for (int i = 0; i < 20; i++) args[i][0] = '\0';
        }

        if (strcmp(cmd, "add_user") == 0) {
            const char *c = get_arg('c');
            const char *u = get_arg('u');
            const char *p = get_arg('p');
            const char *n = get_arg('n');
            const char *m = get_arg('m');
            const char *g = get_arg('g');

            if (u && p && n && m && g) {
                int priv = atoi(g);
                if (add_user(c, u, p, n, m, priv)) {
                    printf("0\n");
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "login") == 0) {
            const char *u = get_arg('u');
            const char *p = get_arg('p');

            if (u && p) {
                if (login(u, p)) {
                    printf("0\n");
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "logout") == 0) {
            const char *u = get_arg('u');

            if (u) {
                if (logout(u)) {
                    printf("0\n");
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "query_profile") == 0) {
            const char *c = get_arg('c');
            const char *u = get_arg('u');

            if (c && u) {
                int c_idx = find_user(c);
                int u_idx = find_user(u);
                if (c_idx != -1 && users[c_idx].online &&
                    (users[c_idx].privilege > users[u_idx].privilege ||
                     strcmp(c, u) == 0)) {
                    printf("%s %s %s %d\n", users[u_idx].username,
                           users[u_idx].name, users[u_idx].mailAddr, users[u_idx].privilege);
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "modify_profile") == 0) {
            const char *c = get_arg('c');
            const char *u = get_arg('u');

            if (c && u) {
                int c_idx = find_user(c);
                int u_idx = find_user(u);
                if (c_idx != -1 && users[c_idx].online &&
                    (users[c_idx].privilege > users[u_idx].privilege ||
                     strcmp(c, u) == 0)) {

                    const char *p = get_arg('p');
                    const char *n = get_arg('n');
                    const char *m = get_arg('m');
                    const char *g = get_arg('g');

                    if (g) {
                        int priv = atoi(g);
                        if (priv >= users[c_idx].privilege) {
                            printf("-1\n");
                            continue;
                        }
                        users[u_idx].privilege = priv;
                    }
                    if (p) strcpy(users[u_idx].password, p);
                    if (n) strcpy(users[u_idx].name, n);
                    if (m) strcpy(users[u_idx].mailAddr, m);

                    printf("%s %s %s %d\n", users[u_idx].username,
                           users[u_idx].name, users[u_idx].mailAddr, users[u_idx].privilege);
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "add_train") == 0) {
            const char *i = get_arg('i');
            const char *n = get_arg('n');
            const char *m = get_arg('m');
            const char *s = get_arg('s');
            const char *p = get_arg('p');
            const char *x = get_arg('x');
            const char *t = get_arg('t');
            const char *o = get_arg('o');
            const char *d = get_arg('d');
            const char *y = get_arg('y');

            if (i && n && m && s && p && x && t && o && d && y) {
                int stationNum = atoi(n);
                int seatNum = atoi(m);
                char type = y[0];
                if (add_train(i, stationNum, seatNum, s, p, x, t, o, d, type)) {
                    printf("0\n");
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "release_train") == 0) {
            const char *i = get_arg('i');

            if (i) {
                if (release_train(i)) {
                    printf("0\n");
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "query_train") == 0) {
            const char *i = get_arg('i');
            const char *d = get_arg('d');

            if (i && d) {
                int month, day;
                sscanf(d, "%d-%d", &month, &day);
                int date = date_to_int(month, day);
                query_train_output(i, date);
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "delete_train") == 0) {
            const char *i = get_arg('i');

            if (i) {
                if (delete_train(i)) {
                    printf("0\n");
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "query_ticket") == 0) {
            const char *s = get_arg('s');
            const char *t = get_arg('t');
            const char *d = get_arg('d');
            const char *p = get_arg('p');

            if (s && t && d) {
                int month, day;
                sscanf(d, "%d-%d", &month, &day);
                int date = date_to_int(month, day);
                const char *priority = (p && strcmp(p, "cost") == 0) ? "cost" : "time";
                query_ticket(s, t, date, priority);
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "query_transfer") == 0) {
            const char *s = get_arg('s');
            const char *t = get_arg('t');
            const char *d = get_arg('d');
            const char *p = get_arg('p');

            if (s && t && d) {
                int month, day;
                sscanf(d, "%d-%d", &month, &day);
                int date = date_to_int(month, day);
                const char *priority = (p && strcmp(p, "cost") == 0) ? "cost" : "time";
                query_transfer(s, t, date, priority);
            } else {
                printf("0\n");
            }
        } else if (strcmp(cmd, "buy_ticket") == 0) {
            const char *u = get_arg('u');
            const char *i = get_arg('i');
            const char *d = get_arg('d');
            const char *n = get_arg('n');
            const char *f = get_arg('f');
            const char *t = get_arg('t');
            const char *q = get_arg('q');

            if (u && i && d && n && f && t) {
                int month, day;
                sscanf(d, "%d-%d", &month, &day);
                int date = date_to_int(month, day);
                int num = atoi(n);
                bool queue = (q && strcmp(q, "true") == 0);

                int result = buy_ticket(u, i, date, num, f, t, queue);
                if (result == -2) {
                    printf("queue\n");
                } else if (result >= 0) {
                    printf("%d\n", result);
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "query_order") == 0) {
            const char *u = get_arg('u');

            if (u) {
                query_order(u);
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "refund_ticket") == 0) {
            const char *u = get_arg('u');
            const char *n = get_arg('n');

            if (u) {
                int num = n ? atoi(n) : 1;
                if (refund_ticket(u, num) == 0) {
                    printf("0\n");
                } else {
                    printf("-1\n");
                }
            } else {
                printf("-1\n");
            }
        } else if (strcmp(cmd, "clean") == 0) {
            clean();
            printf("0\n");
        } else if (strcmp(cmd, "exit") == 0) {
            printf("bye\n");
            break;
        }
    }

    return 0;
}
