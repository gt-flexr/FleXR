#include <raft>
#include <raftio>
#include <cstdio>
#include <type_traits>

using namespace std;

class A : public raft::kernel {
  private:
    int i = 0;
    int cnt = 0;

  public:
    A() : raft::kernel() {
      output.addPort<int>("out");
    }

    virtual raft::kstatus run() {
      i++;
      if(i <= 10) {
        auto c(output["out"].template allocate_s<int>());
        *c = i;
        printf("[ A ] %d, out(%p) \n", ++cnt, static_cast<void*>(&(*c)));
        //cout << "A: " << ++cnt << ":" << *c << " out_addr:" << static_cast<void*>(&(*c)) << endl;
        output["out"].send();
      }

      if(i > 20)
        return raft::stop;

      return raft::proceed;
    }
};

class B : public raft::kernel {
  private:
    int cnt = 0;

  public:
    B() : raft::kernel() {
      input.addPort<int>("in");
      output.addPort<int>("out");
    }

    virtual raft::kstatus run() {
      auto &input_port(this->input["in"]);
      auto &a(input_port.template peek<int>());

      auto c(output["out"].template allocate_s<int>());
      *c = a;
      printf("[ B ] %d, in(%p) out(%p) \n", ++cnt, static_cast<void*>(&a), static_cast<void*>(&(*c)));
      //cout << "B: " << ++cnt << ":" << a << " in_addr:" << static_cast<void*>(&a);
      //cout << " in_addr:" << static_cast<void*>(&a) << endl;
      //cout << " out_addr:" << static_cast<void*>(&(*c)) << endl;
      input_port.recycle(1);

      return raft::proceed;
    }
};

class C : public raft::kernel {
  private:
    int cnt = 0;
  public:
    C() : raft::kernel() {
      input.addPort<int>("in");
    }

    virtual raft::kstatus run() {
      auto &input_port(this->input["in"]);
      auto &a(input_port.template peek<int>());

      //cout << "C: " << ++cnt << ":" << a << " in_addr:" << static_cast<void*>(&a) << endl;
      printf("[ C ] %d, in(%p) \n", ++cnt, static_cast<void*>(&a));
      input_port.recycle();

      return raft::proceed;
    }
};

int main() {
  A a;
  B b;
  C c;

  raft::map m;
  m += a >> b >> c;

  m.exe();

  return 0;
}
