/* group.h
 *
 * Slightly better groupchats implementation.
 *
 *  Copyright (C) 2014 Tox project All Rights Reserved.
 *
 *  This file is part of Tox.
 *
 *  Tox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Tox is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef GROUP_H
#define GROUP_H

#include "Messenger.h"

enum {
    GROUPCHAT_STATUS_NONE,
    GROUPCHAT_STATUS_VALID,
    GROUPCHAT_STATUS_CONNECTED
};

typedef struct {
    uint8_t     real_pk[crypto_box_PUBLICKEYBYTES];
    uint8_t     temp_pk[crypto_box_PUBLICKEYBYTES];

    uint64_t    last_recv;
    uint32_t    last_message_number;

    uint8_t     nick[MAX_NAME_LENGTH];
    uint8_t     nick_len;

    uint16_t peer_number;
} Group_Peer;

#define DESIRED_CLOSE_CONNECTIONS 4
#define MAX_GROUP_CONNECTIONS 16
#define GROUP_IDENTIFIER_LENGTH crypto_box_KEYBYTES /* So we can use new_symmetric_key(...) to fill it */

enum {
    GROUPCHAT_CLOSE_NONE,
    GROUPCHAT_CLOSE_CONNECTION,
    GROUPCHAT_CLOSE_ONLINE
};

typedef struct {
    uint8_t status;

    Group_Peer *group;
    uint32_t numpeers;

    struct {
        uint8_t type; /* GROUPCHAT_CLOSE_* */
        uint8_t closest;
        uint32_t number;
        uint16_t group_number;
    } close[MAX_GROUP_CONNECTIONS];

    uint8_t real_pk[crypto_box_PUBLICKEYBYTES];
    struct {
        uint8_t entry;
        uint8_t real_pk[crypto_box_PUBLICKEYBYTES];
        uint8_t temp_pk[crypto_box_PUBLICKEYBYTES];
    } closest_peers[DESIRED_CLOSE_CONNECTIONS];
    uint8_t changed;

    uint8_t identifier[GROUP_IDENTIFIER_LENGTH];

    uint32_t message_number;
    uint16_t peer_number;

    uint64_t last_sent_ping;

    int number_joined; /* friendcon_id of person that invited us to the chat. (-1 means none) */
} Group_c;

typedef struct {
    Messenger *m;
    Friend_Connections *fr_c;

    Group_c *chats;
    uint32_t num_chats;

    void (*invite_callback)(Messenger *m, int32_t, const uint8_t *, uint16_t, void *);
    void *invite_callback_userdata;
    void (*message_callback)(Messenger *m, int, int, const uint8_t *, uint16_t, void *);
    void *message_callback_userdata;
    void (*action_callback)(Messenger *m, int, int, const uint8_t *, uint16_t, void *);
    void *action_callback_userdata;
    void (*peer_namelistchange)(Messenger *m, int, int, uint8_t, void *);
    void *group_namelistchange_userdata;
} Group_Chats;

/* Set the callback for group invites.
 *
 *  Function(Group_Chats *g_c, int32_t friendnumber, uint8_t *data, uint16_t length, void *userdata)
 *
 *  data of length is what needs to be passed to join_groupchat().
 */
void g_callback_group_invite(Group_Chats *g_c, void (*function)(Messenger *m, int32_t, const uint8_t *, uint16_t,
                             void *), void *userdata);

/* Set the callback for group messages.
 *
 *  Function(Group_Chats *g_c, int groupnumber, int friendgroupnumber, uint8_t * message, uint16_t length, void *userdata)
 */
void g_callback_group_message(Group_Chats *g_c, void (*function)(Messenger *m, int, int, const uint8_t *, uint16_t,
                              void *), void *userdata);

/* Set the callback for group actions.
 *
 *  Function(Group_Chats *g_c, int groupnumber, int friendgroupnumber, uint8_t * message, uint16_t length, void *userdata)
 */
void g_callback_group_action(Group_Chats *g_c, void (*function)(Messenger *m, int, int, const uint8_t *, uint16_t,
                             void *), void *userdata);

/* Set callback function for peer name list changes.
 *
 * It gets called every time the name list changes(new peer/name, deleted peer)
 *  Function(Group_Chats *g_c, int groupnumber, int peernumber, TOX_CHAT_CHANGE change, void *userdata)
 */
enum {
    CHAT_CHANGE_PEER_ADD,
    CHAT_CHANGE_PEER_DEL,
    CHAT_CHANGE_PEER_NAME,
};
void g_callback_group_namelistchange(Group_Chats *g_c, void (*function)(Messenger *m, int, int, uint8_t, void *),
                                     void *userdata);

/* Creates a new groupchat and puts it in the chats array.
 *
 * return group number on success.
 * return -1 on failure.
 */
int add_groupchat(Group_Chats *g_c);

/* Delete a groupchat from the chats array.
 *
 * return 0 on success.
 * return -1 if failure.
 */
int del_groupchat(Group_Chats *g_c, int groupnumber);

/* Copy the name of peernumber who is in groupnumber to name.
 * name must be at least MAX_NAME_LENGTH long.
 *
 * return length of name if success
 * return -1 if failure
 */
int group_peername(const Group_Chats *g_c, int groupnumber, int peernumber, uint8_t *name);

/* invite friendnumber to groupnumber
 * return 0 on success
 * return -1 on failure
 */
int invite_friend(Group_Chats *g_c, int32_t friendnumber, int groupnumber);

/* Join a group (you need to have been invited first.)
 *
 * returns group number on success
 * returns -1 on failure.
 */
int join_groupchat(Group_Chats *g_c, int32_t friendnumber, const uint8_t *data, uint16_t length);

/* send a group message
 * return 0 on success
 * return -1 on failure
 */
int group_message_send(const Group_Chats *g_c, int groupnumber, const uint8_t *message, uint16_t length);

/* send a group action
 * return 0 on success
 * return -1 on failure
 */
int group_action_send(const Group_Chats *g_c, int groupnumber, const uint8_t *action, uint16_t length);

/* Return the number of peers in the group chat on success.
 * return -1 on failure
 */
int group_number_peers(const Group_Chats *g_c, int groupnumber);

/* return 1 if the peernumber corresponds to ours.
 * return 0 on failure.
 */
unsigned int group_peernumber_is_ours(const Group_Chats *g_c, int groupnumber, int peernumber);

/* List all the peers in the group chat.
 *
 * Copies the names of the peers to the name[length][MAX_NAME_LENGTH] array.
 *
 * Copies the lengths of the names to lengths[length]
 *
 * returns the number of peers on success.
 *
 * return -1 on failure.
 */
int group_names(const Group_Chats *g_c, int groupnumber, uint8_t names[][MAX_NAME_LENGTH], uint16_t lengths[],
                uint16_t length);

/* Return the number of chats in the instance m.
 * You should use this to determine how much memory to allocate
 * for copy_chatlist.
 */
uint32_t count_chatlist(Group_Chats *g_c);

/* Copy a list of valid chat IDs into the array out_list.
 * If out_list is NULL, returns 0.
 * Otherwise, returns the number of elements copied.
 * If the array was too small, the contents
 * of out_list will be truncated to list_size. */
uint32_t copy_chatlist(Group_Chats *g_c, int32_t *out_list, uint32_t list_size);

/* Send current name (set in messenger) to all online groups.
 */
void send_name_all_groups(Group_Chats *g_c);

/* Create new groupchat instance. */
Group_Chats *new_groupchats(Messenger *m);

/* main groupchats loop. */
void do_groupchats(Group_Chats *g_c);

/* Free everything related with group chats. */
void kill_groupchats(Group_Chats *g_c);

#endif
