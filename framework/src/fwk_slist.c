/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Intrusive circular singly-linked list.
 */

#include <fwk_assert.h>
#include <fwk_list.h>
#include <fwk_slist.h>

#include <stdbool.h>
#include <stddef.h>

void __fwk_slist_init(struct fwk_slist *list)
{
    fwk_assert(list != NULL);

    list->head = (struct fwk_slist_node *)list;
    list->tail = (struct fwk_slist_node *)list;

#ifdef MARKED_SLIST
    list->mark_slist.current_number_of_elements = 0;
    list->mark_slist.max_number_of_elements = 0;
#endif
}

struct fwk_slist_node *__fwk_slist_head(const struct fwk_slist *list)
{
    fwk_assert(list != NULL);

    if (__fwk_slist_is_empty(list)) {
        return NULL;
    }

    return list->head;
}

bool __fwk_slist_is_empty(const struct fwk_slist *list)
{
    bool is_empty;

    fwk_assert(list != NULL);

    is_empty = list->head == (struct fwk_slist_node *)list;

    if (is_empty) {
        fwk_assert(list->tail == list->head);
    }

    return is_empty;
}

void __fwk_slist_push_head(
    struct fwk_slist *list,
    struct fwk_slist_node *new)
{
    fwk_assert(list != NULL);
    fwk_assert(new != NULL);
    fwk_check(new->next == NULL);

    new->next = list->head;

    list->head = new;
    if (list->tail == (struct fwk_slist_node *)list) {
        list->tail = new;
    }
}

void __fwk_slist_push_tail(
    struct fwk_slist *list,
    struct fwk_slist_node *new)
{
    fwk_assert(list != NULL);
    fwk_assert(new != NULL);
    fwk_check(new->next == NULL);

    new->next = (struct fwk_slist_node *)list;

    list->tail->next = new;
    list->tail = new;
}

#ifdef MARKED_SLIST
void __fwk_slist_push_tail_watch(
    struct fwk_slist *list,
    struct fwk_slist_node *new)
{
    __fwk_slist_push_tail(list, new);
    ++list->mark_slist.current_number_of_elements;
    if (list->mark_slist.max_number_of_elements <
        list->mark_slist.current_number_of_elements) {
        list->mark_slist.max_number_of_elements =
            list->mark_slist.current_number_of_elements;
    }
}
#endif

struct fwk_slist_node *__fwk_slist_pop_head(struct fwk_slist *list)
{
    struct fwk_slist_node *popped;

    fwk_assert(list != NULL);

    if (__fwk_slist_is_empty(list)) {
        return NULL;
    }

    popped = list->head;
    if (popped->next == (struct fwk_slist_node *)list) {
        list->tail = (struct fwk_slist_node *)list;
    }

    list->head = popped->next;

    popped->next = NULL;

    return popped;
}

#ifdef MARKED_SLIST
struct fwk_slist_node *__fwk_slist_pop_head_watch(struct fwk_slist *list)
{
    struct fwk_slist_node *popped = __fwk_slist_pop_head(list);

    if (list->mark_slist.current_number_of_elements > 0) {
        --list->mark_slist.current_number_of_elements;
    }
    return popped;
}
#endif

struct fwk_slist_node *__fwk_slist_next(
    const struct fwk_slist *list,
    const struct fwk_slist_node *node)
{
    fwk_assert(list != NULL);
    fwk_assert(node != NULL);

    fwk_assert(__fwk_slist_contains(list, node));

    return (node->next == (struct fwk_slist_node *)list) ? NULL : node->next;
}

void __fwk_slist_remove(
    struct fwk_slist *list,
    struct fwk_slist_node *node)
{
    fwk_assert(list != NULL);
    fwk_assert(node != NULL);
    fwk_assert(node->next != NULL);

    struct fwk_slist_node *node_iter = (struct fwk_slist_node *)list;

    while (node_iter->next != (struct fwk_slist_node *)list) {
        if (node_iter->next == node) {
            node_iter->next = node->next;

            if (node->next == (struct fwk_slist_node *)list) {
                list->tail = (struct fwk_slist_node *)node_iter;
            }

            node->next = NULL;

            return;
        }
        node_iter = node_iter->next;
    }

    fwk_unexpected();
}

bool __fwk_slist_contains(
    const struct fwk_slist *list,
    const struct fwk_slist_node *node)
{
    const struct fwk_slist_node *node_iter;

    fwk_assert(list != NULL);
    fwk_assert(node != NULL);
    fwk_assert(node->next != NULL);

    node_iter = (struct fwk_slist_node *)list;

    while (node_iter->next != (struct fwk_slist_node *)list) {
        if (node_iter->next == node) {
            return true;
        }

        node_iter = node_iter->next;
    }

    return false;
}

static_assert(offsetof(struct fwk_slist, head) ==
    offsetof(struct fwk_slist_node, next),
    "fwk_slist::head not aligned with fwk_slist_node::next");
