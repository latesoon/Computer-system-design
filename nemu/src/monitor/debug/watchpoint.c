#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

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

void wp_debug(){
  printf("use:");
  WP* temp = head;
  while(temp!= NULL){
    printf("%d ",temp->NO);
    temp = temp->next;
  }
  printf("\nfree:");
  temp = free_;
  while(temp!= NULL){
    printf("%d ",temp->NO);
    temp = temp->next;
  }
  printf("\n");
}

WP* new_wp(){
  if(free_ == NULL)
    return NULL;
  WP* now = free_;
  int no = now->NO;
  free_ = free_->next;
  if(head == NULL){
    now->next = NULL;
    head = now;
    return now;
  }
  if(no < head->NO){
    now->next = head;
    head = now;
    return now;
  }
  WP* temp = head;
  while(temp != NULL){
    if(temp->next == NULL || temp->next->NO > no){
      now->next = temp->next;
      temp->next = now;
      return now;
    }
    temp = temp->next;
  }
  assert(0);
  return NULL;
}

void free_wp(WP* wp){
  //when in this funtion, wp must be valid
  if(head == NULL)
    assert(0);
  if(wp == head)
    head = head->next;
  else{
    WP* tmp = head;
    while(1){
      if(tmp->next == NULL)
        assert(0);
      else if (tmp->next == wp){
        tmp->next = tmp->next->next;
        break;
      }
      else
        tmp = tmp->next;
    }
  }
  if(free_ == NULL){
    wp->next = NULL;
    free_ = wp;
  }
  if(wp->NO < free_->NO){
    wp->next = free_;
    free_ = wp;
  }
  WP* temp = free_;
  while(temp != NULL){
    if(temp->next == NULL || temp->next->NO > wp->NO){
      wp->next = temp->next;
      temp->next = wp;
      break;
    }
    temp = temp->next;
  }
}

bool wp_check(){
  bool update = false;
  WP* temp = head;
  while(temp != NULL){
    bool success = true;
    uint32_t num = expr(temp->exp,&success);
    if(num != temp->val){
      if(!update){
        update = true;
        printf("These watchpoint has been changed:\n");
        printf("No Stored_value           Now_value              Expr\n");
      }
      printf("%2d %010d(0x%08X) %010d(0x%08X) %s\n",temp->NO,temp->val,temp->val,num,num,temp->exp);
    }
    temp = temp -> next;
  }
  return !update;
}

void wp_print(){
  if(head == NULL){
    printf("No watchpoint exists.\n");
    return;
  }
  printf("No Stored_value           Expr\n");
  WP* temp = head;
  while(temp != NULL){
    printf("%2d %010d(0x%08X) %s\n",temp->NO,temp->val,temp->val,temp->exp);
    temp = temp -> next;
  }
}

void wp_add(char* arg){
  bool success = true;
  uint32_t num = expr(arg,&success);
  if(!success){
    printf("\033[1;31mInvalid arg in watchpoint addition!\033[0m\n");
    return;
  }
  WP* new = new_wp();
  if(new == NULL){
    printf("\033[1;31mNo enough watchpoint to be added!\033[0m\n");
    return;
  }
  new->val = num;
  strcpy(new->exp,arg);
  printf("Success! New watchpoint info:\n");
  printf("No Stored_value           Expr\n");
  printf("%2d %010d(0x%08X) %s\n",new->NO,new->val,new->val,new->exp);
  wp_debug();
}

void wp_del(uint32_t no){
  if(no >= NR_WP || head == NULL){
    printf("\033[1;31mInvalid arg in watchpoint deletion!\033[0m\n");
    return;
  }
  bool success = false;
  WP* temp = head;
  while(temp != NULL){
    if(temp->NO == no){
      free_wp(temp);
      success = true;
      printf("Successful watchpoint deletion!\n");
      break;
    }
    temp = temp -> next;
  }
  if(!success)
    printf("\033[1;31mInvalid arg in watchpoint deletion!\033[0m\n");
  wp_debug();
}