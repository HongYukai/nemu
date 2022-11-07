#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

WP* new_wp() {
    assert(free_ != NULL);
    WP *t = free_;
    free_ = free_->next;
    t->next = head;
    head = t;
    return head;
}

WP* get_head() {
    return head;
}

WP* get_wp_pool() {
    return wp_pool;
}

void free_wp(WP *wp) {
    assert(head != NULL && wp != NULL);
    if (wp == head) {
        head = head->next;
        wp->next = free_;
        free_ = wp;
        return;
    }
    WP *t = head;
    while (t != NULL) {
        if (t->next == wp) {
            t->next = wp->next;
            wp->next = free_;
            free_ = wp;
            return;
        }
        t = t->next;
    }
}

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


