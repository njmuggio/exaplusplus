#ifndef EPP_HPP
#define EPP_HPP

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <random>
#include <string>
#include <variant>
#include <vector>

namespace epp
{
struct Error : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

struct MachineFailure : public Error
{
  using Error::Error;
};

using Number = int64_t;
using Value = std::variant<Number, std::string>;

std::ostream& operator<<(std::ostream& rStream, const Value& val);

Value operator+(const Value& left, const Value& right);
Value operator-(const Value& left, const Value& right);
Value operator*(const Value& left, const Value& right);
Value operator/(const Value& left, const Value& right);
Value operator%(const Value& left, const Value& right);
bool operator<(const Value& left, const Value& right);
bool operator==(const Value& left, const Value& right);
bool operator>(const Value& left, const Value& right);

struct Node;
struct HwRegister
{
  HwRegister(const std::string& name, Node* pNode);

  virtual ~HwRegister() = default;

  virtual void write(const Value& val);

  virtual Value read();

  std::string name;

  Node* pHost;
};

struct StdoutRegister : public HwRegister
{
  using HwRegister::HwRegister;

  void write(const Value& val) override;
};

struct StderrRegister : public HwRegister
{
  using HwRegister::HwRegister;



  void write(const Value& val) override;
};

struct StdinRegister : public HwRegister
{
  using HwRegister::HwRegister;

  Value read() override;
};

struct RandRegister : public HwRegister
{
  RandRegister(const std::string& name, Node* pNode, Number seed);

  Value read() override;

  std::mt19937_64 gen;
};

struct FileInRegister : public HwRegister
{
  FileInRegister(const std::string& name, Node* pNode, const std::filesystem::path& file);

  Value read() override;

  std::ifstream stream;
};

struct FileOutRegister : public HwRegister
{
  FileOutRegister(const std::string& name, Node* pNode, const std::filesystem::path& file);

  void write(const Value& val) override;

  std::ofstream stream;
};

struct Instruction
{
  enum class Opcode
  {
    Copy,
    Addi,
    Subi,
    Muli,
    Divi,
    Modi,
    Swiz,
    Jump,
    Tjmp,
    Fjmp,
    Test1,
    TestEq,
    TestGt,
    TestLt,
    Halt,
    Kill,
    Link,
    Host,
    Mode,
    Void,
    Make,
    Grab,
    File,
    Seek,
    Drop,
    Wipe,
    Noop,
    Rand,
    Repl,

    Dump0,
    Dump1,
  };

  enum class Register
  {
    X,
    T,
    M,
    F,
  };

  using Address = size_t;

  using Operand = std::variant<std::monostate, Register, Number, Address, HwRegister*, std::string>;

  Instruction() = default;
  explicit Instruction(Opcode opcode, Operand op1 = Operand{}, Operand op2 = Operand{}, Operand op3 = Operand{});

  Opcode opcode;
  Operand op1;
  Operand op2;
  Operand op3;
};

std::ostream& operator<<(std::ostream& rStream, const Instruction& inst);

std::ostream& operator<<(std::ostream& rStream, const Instruction::Register& reg);

std::ostream& operator<<(std::ostream& rStream, const HwRegister* const hwreg);

std::ostream& operator<<(std::ostream& rStream, const Instruction::Operand& op);

struct File
{
  void initFromDisk(bool readBytes, bool parseInts);

  void writeToDisk();

  bool eof() const;

  Value read();

  void write(const Value& value);

  void voidCurrent();

  void wipe();

  friend std::ostream& operator<<(std::ostream& s, const File& f);

  std::filesystem::path filename;
  std::vector<Value> values;
  uint16_t id = 0;
  bool locked = false;
  bool readonly = false;
  size_t offset = 0;
};

struct Machine
{
  std::unique_ptr<Machine> repl(size_t address);

  bool done() const;

  friend std::ostream& operator<<(std::ostream& s, const Machine& m);

  std::string name;
  Value x;
  Value t;
  std::optional<Value> outM;
  std::optional<File> file;
  std::vector<Instruction> code;
  size_t instPtr = 0;
  size_t replCount = 0;
  bool globalMode = true;
  bool terminated = false;
};

struct Channel
{
  bool send(const Value& value);

  bool available() const;

  std::optional<Value> receive();

  std::optional<Value> val;
};

struct Node
{
  Node() = default;
  Node(const Node&) = delete;
  Node(Node&&) = default;

  Node& operator=(const Node&) = delete;
  Node& operator=(Node&&) = default;

  bool operator==(const std::string& str);

  bool full() const;

  std::string name;
  std::map<int16_t, std::reference_wrapper<Node>> links;
  std::list<std::unique_ptr<Machine>> machines;
  std::vector<std::unique_ptr<Machine>> incomingMachines;
  std::map<uint16_t, File> files;
  std::map<std::string, std::unique_ptr<HwRegister>> registers;
  size_t capacity = std::numeric_limits<size_t>::max();
  Channel localChannel;
};

struct RunStats
{
  size_t size;
  size_t cycles;
  size_t activity;
};

class Network
{
public:
  Network(const std::filesystem::path& path);

  RunStats run();

  friend std::ostream& operator<<(std::ostream& s, const Network& n);

private:
  void processConfigDirective(const std::string& line);

  void processPreprocessorDirective(const std::string& line);

  void processInstruction(const std::string& line);

  void processNoArgs(const std::string& mne);

  void processSingleArg(const std::string& mne, const std::string& op1);

  void processDoubleArg(const std::string& mne, const std::string& op1, const std::string& op2);

  void processTripleArg(const std::string& mne, const std::string& op1, const std::string& op2, const std::string& op3);

  void finalizeActiveMachine();

  Instruction::Operand regOrVal(const std::string& op);

  Instruction::Operand reg(const std::string& op);

  std::optional<Value> get(Node& rNode, Machine& rMachine, const Instruction::Operand& src);

  bool set(Node& rNode, Machine& rMachine, const Instruction::Operand& dest, const Value& val);

  Value clamp(const Value& val);

  static Value swiz(const Value& input, const Value& mask);

  Number rangeMin;
  Number rangeMax;
  
  uint16_t nextFileId;

  std::vector<Node> nodes;
  Channel globalChannel;

  Node* pHomeNode;
  std::map<std::string, HwRegister*> hwRegMap;

  std::unique_ptr<Machine> pMachineBeingAssembled;

  std::map<std::string, Instruction::Address> addressLookup;

  std::vector<std::string> repLines;
  bool addRepLines;
  size_t repCount;

  std::mt19937_64 random;

  RunStats stats;
};
} // namespace epp

#endif // EPP_HPP
