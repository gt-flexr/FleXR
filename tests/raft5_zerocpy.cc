/**
 *
 * Proof of concept Raftlib
 *
 * Want to have a 3 kernels stream which produces and sends 10 numbers down the stream.
 * The 10 numbers should be created by the first kernel and destroyed by the last.
 *
 */
#include <raft>
#include <raftio>

struct big_t
{
  int i;
  std::uintptr_t start;
  char *test;
#ifdef ZEROCPY
  char padding[ 32768 ];
#else
  char padding[ 32 ];
#endif
};



/**
 * Producer: sends down the stream numbers from 1 to 10
 */
class A : public raft::kernel
{
  private:
    int i   = 0;
    int cnt = 0;

  public:
    A() : raft::kernel()
  {
    output.addPort< big_t >("out");
  }

    virtual raft::kstatus run()
    {
      i++;

      if ( i >  -1 )
      {
        auto &c( output["out"].allocate< big_t >() );
        c.i = i;
        c.start = reinterpret_cast< std::uintptr_t >( &(c.i) );
        c.test = new char[4096];
        output["out"].send();
      }

      if ( i < -1 )
      {
        return (raft::stop);
      }
      return (raft::proceed);
    };
};

/**
 * Processor: It simmulates a process with the input numbers
 */
class B : public raft::kernel
{
  private:
    int cnt = 0;
  public:
    B() : raft::kernel()
  {
    input.addPort<big_t>("in");
    output.addPort<big_t>("out");
  }

    virtual raft::kstatus run()
    {
      auto &a( input[ "in" ].peek< big_t >());
      input[ "in" ].recycle( 1 );
      output[ "out" ].push( a );
      return (raft::proceed);
    }
};

/**
 * Consumer: takes the number from input and dumps it to the console
 */
class C : public raft::kernel
{
  private:
    int cnt = 0;
  public:
    C() : raft::kernel()
  {
    input.addPort< big_t >("in");
  }

    virtual raft::kstatus run()
    {
      auto &a( input[ "in" ].peek< big_t >() );
      if(ZEROCPY == 1) {
        std::cout << "ZEROCPY " << std::dec << a.i << " - " << std::hex << a.start << " - " << std::hex <<
          reinterpret_cast< std::uintptr_t >( &a.i ) << "\n";
      }
      else {
        std::cout << std::dec << a.i << " - " << std::hex << a.start << " - " << std::hex <<
          reinterpret_cast< std::uintptr_t >( &a.i ) << "\n";
      }
      delete[] a.test;
      input[ "in" ].recycle(1);
      return (raft::proceed);
    }
};

int main()
{
  A a;
  B b;
  C c;

  raft::map m;

  // m += a >> b; // Core dump
  m += a >> b >> c;

  m.exe();

  return( EXIT_SUCCESS );
}
