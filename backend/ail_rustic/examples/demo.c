struct s {
  int f1;
  int f2 [[rc::cell_scalar]];
  struct s2 * f3 [[rc::cell("f3get", "f3set")]];
};

[[rc::function_lifetime("f")]]
void f([[rc::mut("a")]] struct s * p [[rc::mut("f"), rc::nonnull]]) {
  p->f1 = 1;
}

[[rc::funtion_lifetime("f")]]
void g([[rc::read("f"), rc::inited("s{f1:init,f2:notinit,f3:notinit}")]] struct s * p [[rc::mut("f"), rc::nonnull]]) {
  int x = p->f1 + 1;
}

int f6(void) {
  struct s s0;
  // s0 is nonnull, but not initialised
  f(&s0);
  // s0 now has field f1 initialised
  g(&s0);
}

struct ll_node {
  int f;
  struct ll_node * next [[rc::recursive]];
};

[[rc::function_lifetime("f")]]
void f2(struct ll_node * x [[rc::read("f")]]) {
  if (x) {
    [[rc::block_lifetime("b")]] // now b <= f
    int y = x->f;
    f2(x->next); // this uses b for the f argument
  }
}

// TODO: actual mutex
struct mutex {
  int taken;
};

[[rc::is_lock_function("mutex")]]
void lock(struct mutex *);
[[rc::is_unlock_function("mutex")]]
void unlock(struct mutex *);

struct lk_s {
  struct mutex m [[rc::mutex]];
  int f [[rc::owned_by("m")]];
  int * p [[rc::owned_by("m")]];
  int f2; // not owned
};

void f3(struct lk_s * x) {
  if (x) {
    lock(&x->m);
    int y = x->f;
    if (x->p) {
      int z = *x->p;
    }
    unlock(&x->m);
    // can't access f or p
    int w = x->f2;
  }
}

struct ll_lk_node {
  struct mutex m [[rc::mutex]];
  int f [[rc::owned_by("m")]];
  struct ll_lk_node * next [[rc::recursive, rc::owned_by("m")]];
};

void f4() {
  int x; // owns something of size |int|
  int y = x + 1;
}

[[rc::function_lifetime("f")]]
void f5(int * x [[rc::mut("f")]]) { // rc::mut should implicitly mean rc::mut("f")
  [[rc::block_lifetime("b")]]
  [[rc::updatety("x","*zap")]]
  // after the following statement, one of `x` and `y` has to be a pointer what `x` was a pointer to
  // the other's value should not be "used" (or at least, not used to dereference)
  int * y = x; // y has lifetime b
  int z = *y; // it has to be `y`
  // y should ???
}

[[rc::function_lifetime("f")]]
void f7_free(struct ll_lk_node * s [[rc::mine]]) {
  // delete s is fine
}

void f8_call_f7(void) {
  struct ll_lk_node * s; //
  f7_free(s);
}