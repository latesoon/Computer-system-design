#include "cpu/exec.h"

make_EHelper(test) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t2, id_dest->width); 

  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t2, id_dest->width); 

  print_asm_template2(and);
}

make_EHelper(xor) {
  //TODO();
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t2, id_dest->width);  

  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t2, id_dest->width);  

  print_asm_template2(or);
}

make_EHelper(sar) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_sext(&t2, &id_dest->val, id_dest->width);
  rtl_sar(&t0, &t2, &id_src->val);
  rtl_update_ZFSF(&t0, id_dest->width);  
  operand_write(id_dest, &t0);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t0, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t0, id_dest->width);  
  operand_write(id_dest, &t0);
  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t0, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t0, id_dest->width);  
  operand_write(id_dest, &t0);
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //TODO();
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}

//ADD
make_EHelper(rol){
  int shift = id_src->val % (id_dest->width * 8);
  rtl_shri(&t2, &id_dest->val, id_dest->width * 8 - shift);
	rtl_shli(&t1, &id_dest->val, shift);
	rtl_or(&t1, &t2, &t1);

  rtl_msb(&t3,&id_dest->val, id_dest->width);
  operand_write(id_dest, &t1);

  rtl_msb(&t0, &id_dest->val, id_dest->width);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &t0, &t3);
  rtl_set_OF(&t0);
  print_asm_template1(rol);
}
