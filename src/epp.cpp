#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>

#include "epp.hpp"

namespace epp
{
std::ostream& operator<<(std::ostream& rStream, const Value& val)
{
  std::visit([&](const auto& arg)
    {
      rStream << arg;
    }, val);

  return rStream;
}

Value operator+(const Value& left, const Value& right)
{
  if (std::holds_alternative<Number>(left) && std::holds_alternative<Number>(right))
  {
    return std::get<Number>(left) + std::get<Number>(right);
  }
  else
  {
    throw MachineFailure("Tried to do arithmetic with a string");
  }
}

Value operator-(const Value& left, const Value& right)
{
  if (std::holds_alternative<Number>(left) && std::holds_alternative<Number>(right))
  {
    return std::get<Number>(left) - std::get<Number>(right);
  }
  else
  {
    throw MachineFailure("Tried to do arithmetic with a string");
  }
}

Value operator*(const Value& left, const Value& right)
{
  if (std::holds_alternative<Number>(left) && std::holds_alternative<Number>(right))
  {
    return std::get<Number>(left) * std::get<Number>(right);
  }
  else
  {
    throw MachineFailure("Tried to do arithmetic with a string");
  }
}

Value operator/(const Value& left, const Value& right)
{
  if (std::holds_alternative<Number>(left) && std::holds_alternative<Number>(right))
  {
    if (std::get<Number>(right) == 0)
    {
      throw MachineFailure("Tried to divide by zero");
    }

    return std::get<Number>(left) / std::get<Number>(right);
  }
  else
  {
    throw MachineFailure("Tried to do arithmetic with a string");
  }
}

Value operator%(const Value& left, const Value& right)
{
  if (std::holds_alternative<Number>(left) && std::holds_alternative<Number>(right))
  {
    if (std::get<Number>(right) == 0)
    {
      throw MachineFailure("Tried to divide by zero");
    }

    return std::get<Number>(left) % std::get<Number>(right);
  }
  else
  {
    throw MachineFailure("Tried to do arithmetic with a string");
  }
}

bool operator<(const Value& left, const Value& right)
{
  if (left.index() != right.index())
  {
    return false;
  }

  return std::visit([&](const auto& arg)
    {
      return arg < right;
    }, left);
}

bool operator==(const Value& left, const Value& right)
{
  if (left.index() != right.index())
  {
    return false;
  }

  return std::visit([&](const auto& arg)
    {
      return arg == right;
    }, left);
}

bool operator>(const Value& left, const Value& right)
{
  if (left.index() != right.index())
  {
    return false;
  }

  return std::visit([&](const auto& arg)
    {
      return arg > right;
    }, left);
}

HwRegister::HwRegister(const std::string& name, Node* pNode)
  : name(name),
    pHost(pNode)
{
  // Empty
}

void HwRegister::write(const Value& val)
{
  // Do nothing
}

Value HwRegister::read()
{
  return 0;
}

void StdoutRegister::write(const Value& val)
{
  std::cout << val;
}

void StderrRegister::write(const Value& val)
{
  std::cerr << val;
}

Value StdinRegister::read()
{
  Value ret;
  std::string s;
  std::cin >> s;
  ret = s;

  try
  {
    Number num = std::stol(s);
    ret = num;
  }
  catch (const std::exception&)
  {
    // Ignored
  }

  return ret;
}

RandRegister::RandRegister(const std::string& name, Node* pNode, Number seed)
  : HwRegister(name, pNode),
    gen(seed)
{
  // Empty
}

Value RandRegister::read()
{
  uint64_t bits = gen();
  int64_t val = 0;
  std::memcpy(&val, &bits, sizeof(val));
  return val;
}

FileInRegister::FileInRegister(const std::string& name, Node* pNode, const std::filesystem::path& file)
  : HwRegister(name, pNode),
    stream(file)
{
  // Empty
}

Value FileInRegister::read()
{
  Value ret = 0;

  if (stream)
  {
    std::string s;
    stream >> s;
    ret = s;

    try
    {
      Number num = std::stol(s);
      ret = num;
    }
    catch (const std::exception&)
    {
      // Ignored
    }
  }

  return ret;
}

FileOutRegister::FileOutRegister(const std::string& name, Node* pNode, const std::filesystem::path& file)
  : HwRegister(name, pNode),
    stream(file)
{
  // Empty
}

void FileOutRegister::write(const Value& val)
{
  stream << val;
}

std::ostream& operator<<(std::ostream& rStream, const Instruction& inst)
{
  switch (inst.opcode)
  {
    case Instruction::Opcode::Copy:
      rStream << "COPY " << inst.op1 << ' ' << inst.op2;
      break;
    case Instruction::Opcode::Addi:
      rStream << "ADDI " << inst.op1 << ' ' << inst.op2 << ' ' << inst.op3;
      break;
    case Instruction::Opcode::Subi:
      rStream << "SUBI " << inst.op1 << ' ' << inst.op2 << ' ' << inst.op3;
      break;
    case Instruction::Opcode::Muli:
      rStream << "MULI " << inst.op1 << ' ' << inst.op2 << ' ' << inst.op3;
      break;
    case Instruction::Opcode::Divi:
      rStream << "DIVI " << inst.op1 << ' ' << inst.op2 << ' ' << inst.op3;
      break;
    case Instruction::Opcode::Modi:
      rStream << "MODI " << inst.op1 << ' ' << inst.op2 << ' ' << inst.op3;
      break;
    case Instruction::Opcode::Swiz:
      rStream << "SWIZ " << inst.op1 << ' ' << inst.op2 << ' ' << inst.op3;
      break;
    case Instruction::Opcode::Jump:
      rStream << "JUMP " << inst.op1;
      break;
    case Instruction::Opcode::Tjmp:
      rStream << "TJMP " << inst.op1;
      break;
    case Instruction::Opcode::Fjmp:
      rStream << "FJMP " << inst.op1;
      break;
    case Instruction::Opcode::Test1:
      rStream << "TEST " << inst.op1;
      break;
    case Instruction::Opcode::TestEq:
      rStream << "TEST " << inst.op1 << " = " << inst.op2;
      break;
    case Instruction::Opcode::TestGt:
      rStream << "TEST " << inst.op1 << " > " << inst.op2;
      break;
    case Instruction::Opcode::TestLt:
      rStream << "TEST " << inst.op1 << " < " << inst.op2;
      break;
    case Instruction::Opcode::Halt:
      rStream << "HALT";
      break;
    case Instruction::Opcode::Kill:
      rStream << "KILL";
      break;
    case Instruction::Opcode::Link:
      rStream << "LINK " << inst.op1;
      break;
    case Instruction::Opcode::Host:
      rStream << "HOST " << inst.op1;
      break;
    case Instruction::Opcode::Mode:
      rStream << "MODE";
      break;
    case Instruction::Opcode::Void:
      rStream << "VOID " << inst.op1;
      break;
    case Instruction::Opcode::Make:
      rStream << "MAKE";
      break;
    case Instruction::Opcode::Grab:
      rStream << "GRAB " << inst.op1;
      break;
    case Instruction::Opcode::File:
      rStream << "FILE " << inst.op1;
      break;
    case Instruction::Opcode::Seek:
      rStream << "SEEK " << inst.op1;
      break;
    case Instruction::Opcode::Drop:
      rStream << "DROP";
      break;
    case Instruction::Opcode::Wipe:
      rStream << "WIPE";
      break;
    case Instruction::Opcode::Noop:
      rStream << "NOOP";
      break;
    case Instruction::Opcode::Rand:
      rStream << "RAND " << inst.op1;
      break;
    case Instruction::Opcode::Repl:
      rStream << "REPL " << inst.op1;
      break;
    case Instruction::Opcode::Dump0:
      rStream << "DUMP";
      break;
    case Instruction::Opcode::Dump1:
      rStream << "DUMP " << inst.op1;
      break;
  }

  return rStream;
}

std::ostream& operator<<(std::ostream& rStream, const Instruction::Register& reg)
{
  switch (reg)
  {
    case Instruction::Register::X:
      rStream << "X";
      break;
    case Instruction::Register::T:
      rStream << "T";
      break;
    case Instruction::Register::M:
      rStream << "M";
      break;
    case Instruction::Register::F:
      rStream << "F";
  }

  return rStream;
}

std::ostream& operator<<(std::ostream& rStream, const HwRegister* const hwreg)
{
  rStream << hwreg->name;
  return rStream;
}

std::ostream& operator<<(std::ostream& rStream, const Instruction::Operand& op)
{
  std::visit([&](const auto& arg)
    {
      rStream << arg;
    }, op);

  return rStream;
}

void File::initFromDisk(bool readBytes, bool parseInts)
{
  std::ifstream stream(filename);

  while (stream)
  {
    if (readBytes)
    {
      char c{};
      stream.get(c);
      values.emplace_back(Number(c));
    }
    else
    {
      Value val;
      std::string s;
      stream >> s;
      val = s;

      if (parseInts)
      {
        try
        {
          Number num = std::stol(s);
          val = num;
        }
        catch (const std::exception&)
        {
          // Ignored
        }
      }

      values.emplace_back(std::move(val));
    }
  }
}

void File::writeToDisk()
{
  std::ofstream stream(filename);

  for (const auto& val : values)
  {
    stream << val << '\n';
  }
}

bool File::eof() const
{
  return offset >= values.size();
}

Value File::read()
{
  if (offset >= values.size())
  {
    throw MachineFailure("Tried to read past end of file");
  }

  return values[offset++];
}

void File::write(const Value& value)
{
  if (offset < values.size())
  {
    values[offset] = value;
  }
  else
  {
    values.push_back(value);
    offset = values.size();
  }
}

void File::voidCurrent()
{
  if (offset < values.size())
  {
    values.erase(values.begin() + offset);
  }
  else
  {
    throw MachineFailure("Tried to void past end of file");
  }
}

void File::wipe()
{
  values.clear();
  offset = 0;
}

std::ostream& operator<<(std::ostream& s, const File& f)
{
  s << "File{filename=" << f.filename << "; id=" << f.id << "; locked=" << f.locked << "; readonly=" << f.readonly << "; offset=" << f.offset;
  s << "; content={";

  for (const auto& val : f.values)
  {
    s << val << "; ";
  }
  s << "}}";

  return s;
}

std::unique_ptr<Machine> Machine::repl(size_t address)
{
  auto pRet = std::make_unique<Machine>();
  pRet->name = name + ":" + std::to_string(replCount++);
  pRet->x = x;
  pRet->t = t;
  pRet->code = code;
  pRet->instPtr = address;
  pRet->replCount = 0;
  pRet->globalMode = globalMode;
  pRet->terminated = false;
  return pRet;
}

bool Machine::done() const
{
  return instPtr >= code.size();
}

std::ostream& operator<<(std::ostream& s, const Machine& m)
{
  s << "Machine{name=" << m.name << "; x=" << m.x << "; t=" << m.t << "; file=";

  if (m.file)
  {
    s << "{" << *m.file << "}";
  }
  else
  {
    s << "<none>";
  }

  s << "; instPtr=" << m.instPtr << '}';

  return s;
}

Instruction::Instruction(Opcode opcode, Operand op1, Operand op2, Operand op3)
  : opcode(opcode),
    op1(op1),
    op2(op2),
    op3(op3)
{
  // Empty
}

bool Channel::send(const Value& value)
{
  if (val.has_value())
  {
    return false;
  }

  val = value;
  return true;
}

bool Channel::available() const
{
  return val.has_value();
}

std::optional<Value> Channel::receive()
{
  return val;
}

bool Node::operator==(const std::string& str)
{
  return name == str;
}

bool Node::full() const
{
  return machines.size() + files.size() + incomingMachines.size() >= capacity;
}

Network::Network(const std::filesystem::path& path)
  : rangeMin(-9999),
  rangeMax(9999),
  nextFileId(400),
  nodes(),
  globalChannel(),
  pHomeNode(),
  hwRegMap(),
  pMachineBeingAssembled(),
  addressLookup(),
  repLines(),
  addRepLines(false),
  repCount(0),
  random(4604955068226825093l),
  stats()
{
  std::ifstream stream(path);
  size_t lineno = 0;

  while (stream)
  {
    ++lineno;
    std::string line;
    std::getline(stream, line);

    // Remove comments
    {
      size_t commentStart = line.find(';');
      if (commentStart != std::string::npos)
      {
        line = line.substr(0, commentStart);
      }

      std::string upperLine(line);
      std::transform(upperLine.begin(), upperLine.end(), upperLine.begin(), ::toupper);

      commentStart = upperLine.find("NOTE");
      if (commentStart != std::string::npos)
      {
        line = line.substr(0, commentStart);
      }
    }

    size_t firstValid = 0;

    for (size_t i = 0; i < line.size(); i++)
    {
      if (!std::isblank(line[i]))
      {
        firstValid = i;
        break;
      }
    }

    if (firstValid > 0)
    {
      line.erase(0, firstValid);
    }

    size_t lastValid = 0;

    for (size_t i = 0; i < line.size(); i++)
    {
      if (!std::isblank(line[i]))
      {
        lastValid = i;
      }
    }

    if (lastValid < line.size())
    {
      line.erase(lastValid + 1, line.size() - lastValid);
    }

    if (line.empty())
    {
      continue;
    }

    // Handle configuration directives
    if (line[0] == '.')
    {
      processConfigDirective(line);
    }
    else
    {
      if (!pMachineBeingAssembled)
      {
        throw Error("Encountered instruction before .start command");
      }

      std::transform(line.begin(), line.end(), line.begin(), ::tolower);

      if (line[0] == '@')
      {
        processPreprocessorDirective(line);
      }
      else
      {
        if (addRepLines)
        {
          repLines.push_back(line);
        }
        else
        {
          processInstruction(line);
        }
      }
    }
  }

  finalizeActiveMachine();
}

RunStats Network::run()
{
  size_t machinesRemaining = 0;
  do
  {
    stats.cycles++;
    machinesRemaining = 0;

    for (auto& rNode : nodes)
    {
      for (auto& rpMachine : rNode.machines)
      {
        bool advance = true;

        try
        {
          if (rpMachine->instPtr >= rpMachine->code.size())
          {
            throw MachineFailure("No more instructions");
          }

          const Instruction& inst = rpMachine->code[rpMachine->instPtr];

          switch (inst.opcode)
          {
            case Instruction::Opcode::Copy:
            {
              std::optional<Value> val = get(rNode, *rpMachine, inst.op1);
              advance = val.has_value() && set(rNode, *rpMachine, inst.op2, val.value());
              break;
            }
            case Instruction::Opcode::Addi:
            {
              std::optional<Value> left = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> right = get(rNode, *rpMachine, inst.op2);
              advance = left && right && set(rNode, *rpMachine, inst.op3, *left + *right);
              break;
            }
            case Instruction::Opcode::Subi:
            {
              std::optional<Value> left = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> right = get(rNode, *rpMachine, inst.op2);
              advance = left && right && set(rNode, *rpMachine, inst.op3, *left - *right);
              break;
            }
            case Instruction::Opcode::Muli:
            {
              std::optional<Value> left = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> right = get(rNode, *rpMachine, inst.op2);
              advance = left && right && set(rNode, *rpMachine, inst.op3, *left * *right);
              break;
            }
            case Instruction::Opcode::Divi:
            {
              std::optional<Value> left = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> right = get(rNode, *rpMachine, inst.op2);
              advance = left && right && set(rNode, *rpMachine, inst.op3, *left / *right);
              break;
            }
            case Instruction::Opcode::Modi:
            {
              std::optional<Value> left = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> right = get(rNode, *rpMachine, inst.op2);
              advance = left && right && set(rNode, *rpMachine, inst.op3, *left % *right);
              break;
            }
            case Instruction::Opcode::Swiz:
            {
              std::optional<Value> input = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> mask = get(rNode, *rpMachine, inst.op2);

              if (input && mask)
              {
                Value swizzed = swiz(*input, *mask);
                advance = set(rNode, *rpMachine, inst.op3, swizzed);
              }
              else
              {
                advance = false;
              }

              break;
            }
            case Instruction::Opcode::Jump:
            {
              if (std::holds_alternative<Instruction::Address>(inst.op1))
              {
                rpMachine->instPtr = std::get<Instruction::Address>(inst.op1);
                advance = false;
              }
              else
              {
                throw Error("Jump address is incorrect type");
              }

              break;
            }
            case Instruction::Opcode::Tjmp:
            {
              if (std::holds_alternative<Instruction::Address>(inst.op1))
              {
                if (std::holds_alternative<std::string>(rpMachine->t) || (std::holds_alternative<Number>(rpMachine->t) && std::get<Number>(rpMachine->t) != 0))
                {
                  rpMachine->instPtr = std::get<Instruction::Address>(inst.op1);
                  advance = false;
                }
              }
              else
              {
                throw Error("Jump address is incorrect type");
              }

              break;
            }
            case Instruction::Opcode::Fjmp:
            {
              if (std::holds_alternative<Instruction::Address>(inst.op1))
              {
                if (std::holds_alternative<Number>(rpMachine->t) && std::get<Number>(rpMachine->t) == 0)
                {
                  rpMachine->instPtr = std::get<Instruction::Address>(inst.op1);
                  advance = false;
                }
              }
              else
              {
                throw Error("Jump address is incorrect type");
              }

              break;
            }
            case Instruction::Opcode::Test1:
            {
              if (!std::holds_alternative<Instruction::Register>(inst.op1))
              {
                throw Error("Test EOF/MRD does not reference register");
              }

              Instruction::Register reg = std::get<Instruction::Register>(inst.op1);

              if (reg == Instruction::Register::M)
              {
                Channel* pChannel = rpMachine->globalMode ? &globalChannel : &rNode.localChannel;
                rpMachine->t = pChannel->available() ? 1 : 0;
              }
              else if (reg == Instruction::Register::F)
              {
                if (rpMachine->file.has_value())
                {
                  rpMachine->t = rpMachine->file->eof() ? 1 : 0;
                }
                else
                {
                  throw MachineFailure("Tried to check for EOF, but no file held");
                }
              }
              else
              {
                throw Error("Test EOF/MRD references invalid register");
              }

              break;
            }
            case Instruction::Opcode::TestEq:
            {
              std::optional<Value> left = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> right = get(rNode, *rpMachine, inst.op2);

              if (left && right)
              {
                rpMachine->t = *left == *right ? 1 : 0;
              }
              else
              {
                advance = false;
              }

              break;
            }
            case Instruction::Opcode::TestGt:
            {
              std::optional<Value> left = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> right = get(rNode, *rpMachine, inst.op2);

              if (left && right)
              {
                rpMachine->t = *left > *right ? 1 : 0;
              }
              else
              {
                advance = false;
              }

              break;
            }
            case Instruction::Opcode::TestLt:
            {
              std::optional<Value> left = get(rNode, *rpMachine, inst.op1);
              std::optional<Value> right = get(rNode, *rpMachine, inst.op2);

              if (left && right)
              {
                rpMachine->t = *left < *right ? 1 : 0;
              }
              else
              {
                advance = false;
              }
              break;
            }
            case Instruction::Opcode::Halt:
            {
              throw MachineFailure("Halted");
              break;
            }
            case Instruction::Opcode::Kill:
            {
              stats.activity++;

              if (rNode.machines.size() > 1)
              {
                std::uniform_int_distribution<size_t> dist(0, rNode.machines.size() - 2);
                size_t target = dist(random);

                int curIdx = 0;
                for (auto& rpTarget: rNode.machines)
                {
                  if (rpTarget == rpMachine)
                  {
                    curIdx--;
                  }

                  if (curIdx == target)
                  {
                    rpTarget->terminated = true;
                    break;
                  }
                }
              }
              
              break;
            }
            case Instruction::Opcode::Link:
            {
              std::optional<Value> dest = get(rNode, *rpMachine, inst.op1);

              if (dest)
              {
                if (!std::holds_alternative<Number>(*dest))
                {
                  throw MachineFailure("Cannot link to a string");
                }

                auto iter = rNode.links.find(static_cast<uint16_t>(std::get<Number>(inst.op1)));

                if (iter == rNode.links.end())
                {
                  throw MachineFailure("Link does not exist");
                }

                if (iter->second.get().full())
                {
                  advance = false;
                }
                else
                {
                  stats.activity++;
                  rpMachine->instPtr++;
                  iter->second.get().incomingMachines.emplace_back(std::move(rpMachine));
                }
              }
              else
              {
                advance = false;
              }

              break;
            }
            case Instruction::Opcode::Host:
            {
              advance = set(rNode, *rpMachine, inst.op1, rNode.name);
              break;
            }
            case Instruction::Opcode::Mode:
            {
              rpMachine->globalMode = !rpMachine->globalMode;
              break;
            }
            case Instruction::Opcode::Void:
            {
              if (!std::holds_alternative<Instruction::Register>(inst.op1))
              {
                throw Error("Void does not reference register");
              }

              Instruction::Register reg = std::get<Instruction::Register>(inst.op1);

              if (reg == Instruction::Register::M)
              {
                std::optional<Value> discard = get(rNode, *rpMachine, inst.op1);
                advance = discard.has_value();
              }
              else if (reg == Instruction::Register::F)
              {
                // Voiding past EOF kills exa
                if (rpMachine->file.has_value())
                {
                  rpMachine->file->voidCurrent();
                }
                else
                {
                  throw MachineFailure("Tried to void file, but no file held");
                }
              }
              else
              {
                throw Error("Void references invalid register");
              }

              break;
            }
            case Instruction::Opcode::Make:
            {
              if (rpMachine->file.has_value())
              {
                throw MachineFailure("Tried to make, but already holding file");
              }

              rpMachine->file = File();
              rpMachine->file->id = nextFileId++;
              rpMachine->file->filename = std::to_string(rpMachine->file->id) + ".txt";

              break;
            }
            case Instruction::Opcode::Grab:
            {
              std::optional<Value> fileId = get(rNode, *rpMachine, inst.op1);

              if (fileId)
              {
                if (std::holds_alternative<Number>(*fileId))
                {
                  auto iter = rNode.files.find(static_cast<uint16_t>(std::get<Number>(fileId.value())));

                  if (iter == rNode.files.end())
                  {
                    throw MachineFailure("Tried to grab nonexistent file");
                  }

                  rpMachine->file.emplace(std::move(iter->second));

                  rNode.files.erase(iter);
                }
                else
                {
                  throw MachineFailure("Tried to grab file with string name");
                }
              }
              else
              {
                advance = false;
              }

              break;
            }
            case Instruction::Opcode::File:
            {
              if (rpMachine->file)
              {
                advance = set(rNode, *rpMachine, inst.op1, rpMachine->file->id);
              }
              else
              {
                throw MachineFailure("Cannot get file ID: no file held");
              }

              break;
            }
            case Instruction::Opcode::Seek:
            {
              if (rpMachine->file)
              {
                std::optional<Value> offset = get(rNode, *rpMachine, inst.op1);
                if (offset)
                {
                  if (std::holds_alternative<Number>(*offset))
                  {
                    rpMachine->file->offset += std::get<Number>(*offset);
                  }
                  else
                  {
                    throw MachineFailure("Cannot seek: offset is a string");
                  }
                }
                else
                {
                  advance = false;
                }
              }
              else
              {
                throw MachineFailure("Cannot seek: no file held");
              }

              break;
            }
            case Instruction::Opcode::Drop:
            {
              if (rpMachine->file)
              {
                if (!rNode.full())
                {
                  uint16_t id = rpMachine->file->id;
                  rNode.files.emplace(id, std::move(*rpMachine->file));
                  rpMachine->file.reset();
                }
                else
                {
                  advance = false;
                }
              }
              else
              {
                throw MachineFailure("Cannot drop: no file held");
              }

              break;
            }
            case Instruction::Opcode::Wipe:
            {
              if (rpMachine->file)
              {
                rpMachine->file->wipe();
              }
              else
              {
                throw MachineFailure("Cannot wipe: no file held");
              }

              break;
            }
            case Instruction::Opcode::Noop:
            {
              // Do nothing
              break;
            }
            case Instruction::Opcode::Rand:
            {
              uint64_t bits = random();
              int64_t val = 0;
              std::memcpy(&val, &bits, sizeof(val));
              advance = set(rNode, *rpMachine, inst.op1, val);
              break;
            }
            case Instruction::Opcode::Repl:
            {
              if (!std::holds_alternative<Instruction::Address>(inst.op1))
              {
                throw Error("Repl did not refer to code address");
              }

              if (!rNode.full())
              {
                rNode.incomingMachines.emplace_back(rpMachine->repl(std::get<Instruction::Address>(inst.op1)));
              }
              else
              {
                advance = false;
              }

              break;
            }
            case Instruction::Opcode::Dump0:
            {
              std::cout << *this << '\n';
              break;
            }
            case Instruction::Opcode::Dump1:
            {
              if (!std::holds_alternative<std::string>(inst.op1))
              {
                throw Error("Dump did not have string param");
              }

              std::string s(std::get<std::string>(inst.op1));

              if (s == "me")
              {
                std::cout << *rpMachine << '\n';
              }
              else if (s == "code")
              {
                std::cout << "Code:[";

                for (size_t i = 0; i < rpMachine->code.size(); i++)
                {
                  std::cout << rpMachine->code[i];

                  if (i < rpMachine->code.size() - 1)
                  {
                    std::cout << "; ";
                  }
                }

                std::cout << "]\n";
              }
              else
              {
                throw Error("Unrecognized dump argument: " + s);
              }

              break;
            }
          }
        }
        catch (const MachineFailure& e)
        {
          rpMachine->terminated = true;
          std::cerr << rpMachine->name << ": " << e.what() << '\n';
        }

        if (advance && rpMachine)
        {
          rpMachine->instPtr++;
        }
      }

      for (auto& rpMachine : rNode.machines)
      {
        if (!rpMachine || !rpMachine->terminated)
        {
          continue;
        }

        if (rpMachine->file)
        {
          uint16_t id = rpMachine->file->id;
          rNode.files.emplace(id, std::move(*rpMachine->file));
        }
      }

      auto endIter = std::remove_if(rNode.machines.begin(), rNode.machines.end(),
        [&](const std::unique_ptr<Machine>& pMachine)
        {
          return !pMachine || pMachine->terminated;
        });

      rNode.machines.erase(endIter, rNode.machines.end());
    }

    for (auto& rNode : nodes)
    {
      for (auto& rpMachine : rNode.incomingMachines)
      {
        rNode.machines.push_back(std::move(rpMachine));
      }

      rNode.incomingMachines.clear();

      machinesRemaining += rNode.machines.size();
    }
  } while (machinesRemaining > 0);

  for (auto& rNode : nodes)
  {
    for (auto& rPair : rNode.files)
    {
      rPair.second.writeToDisk();
    }
  }

  return stats;
}

std::ostream& operator<<(std::ostream& s, const Network& n)
{
  s << "TODO";

  /*for (const auto& node : nodes)
  {
    s << node << '\n';
  }

  s << "  globalChannel: "*/

  return s;
}

void Network::processConfigDirective(const std::string& line)
{
  static std::regex rangeStmt(R"r(\.range (-?\d+) (-?\d+))r", std::regex_constants::ECMAScript | std::regex_constants::icase);
  static std::regex nodeStmt(R"r(\.node (\w+)(?: (\d+))?)r", std::regex_constants::ECMAScript | std::regex_constants::icase);
  static std::regex linkStmt(R"r(\.link \((\w+) (-?\d+)\) \((\w+)(?: (-?\d+))?\))r", std::regex_constants::ECMAScript | std::regex_constants::icase);
  static std::regex fileStmt(R"r(\.file "(.*)" (\w+) (\d+) (rw|ro) (word|byte) (noint|int)(?: (locked))?)r", std::regex_constants::ECMAScript | std::regex_constants::icase);
  static std::regex regStmt(R"r(\.reg (sink|file_out|file_in|rand|stdin|stdout|stderr) (#[A-Z]+) (\w+)(?: "?(.*)"?)?)r", std::regex_constants::ECMAScript | std::regex_constants::icase);
  static std::regex startStmt(R"r(\.start (\w+))r", std::regex_constants::ECMAScript | std::regex_constants::icase);
  static std::regex homeStmt(R"r(\.home (\w+))r", std::regex_constants::ECMAScript | std::regex_constants::icase);

  std::smatch match;

  if (std::regex_match(line, match, rangeStmt))
  {
    rangeMin = std::stol(match[1]);
    rangeMax = std::stol(match[2]);
  }
  else if (std::regex_match(line, match, nodeStmt))
  {
    Node node;
    node.name = match[1];

    if (match[2].matched)
    {
      node.capacity = std::stoul(match[2]);
    }
    else
    {
      node.capacity = std::numeric_limits<size_t>::max();
    }

    nodes.emplace_back(std::move(node));
  }
  else if (std::regex_match(line, match, linkStmt))
  {
    auto fromNode = std::find(nodes.begin(), nodes.end(), match[1].str());

    if (fromNode == nodes.end())
    {
      throw Error("Tried to link from unknown node");
    }

    auto toNode = std::find(nodes.begin(), nodes.end(), match[3].str());

    if (toNode == nodes.end())
    {
      throw Error("Tried to link to unknown node");
    }

    int16_t fromNum = static_cast<int16_t>(std::stoi(match[2]));

    bool inserted = fromNode->links.emplace(fromNum, *toNode).second;
    if (!inserted)
    {
      throw Error("Tried to replace existing link");
    }

    if (match[4].matched)
    {
      int16_t toNum = static_cast<int16_t>(std::stoi(match[4]));
      inserted = toNode->links.emplace(toNum, *fromNode).second;
      if (!inserted)
      {
        throw Error("Tried to replace existing link");
      }
    }
  }
  else if (std::regex_match(line, match, fileStmt))
  {
    File file;

    auto node = std::find(nodes.begin(), nodes.end(), match[2].str());

    if (node == nodes.end())
    {
      throw Error("Tried to add file to unknown node");
    }

    file.filename = std::filesystem::absolute(match[1].str());
    file.id = static_cast<uint16_t>(std::stoul(match[3]));
    file.readonly = "ro" == match[4];
    file.locked = match[7].matched;

    file.initFromDisk("byte" == match[5], "int" == match[6]);

    if (node->machines.size() + node->files.size() < node->capacity)
    {
      node->files.emplace(file.id, file);
    }
    else
    {
      throw Error("Tried to add file to node, but node is already full");
    }
  }
  else if (std::regex_match(line, match, regStmt))
  {
    auto node = std::find(nodes.begin(), nodes.end(), match[3].str());
    if (node == nodes.end())
    {
      throw Error("Tried to add hardware register to unknown node");
    }

    auto existingRegister = node->registers.find(match[2]);
    if (existingRegister != node->registers.end())
    {
      throw Error("Tried to add duplicate hardware register");
    }

    if (match[1] == "sink")
    {
      node->registers[match[2]] = std::make_unique<HwRegister>(match[2], &*node);
    }
    else if (match[1] == "stdin")
    {
      node->registers[match[2]] = std::make_unique<StdinRegister>(match[2], &*node);
    }
    else if (match[1] == "stdout")
    {
      node->registers[match[2]] = std::make_unique<StdoutRegister>(match[2], &*node);
    }
    else if (match[1] == "stderr")
    {
      node->registers[match[2]] = std::make_unique<StderrRegister>(match[2], &*node);
    }
    else if (match[1] == "rand")
    {
      if (!match[4].matched)
      {
        throw Error("Tried to create rand register without seed");
      }

      node->registers[match[2]] = std::make_unique<RandRegister>(match[2], &*node, std::stol(match[4]));
    }
    else if (match[1] == "file_in")
    {
      if (!match[4].matched)
      {
        throw Error("Tried to create file_in register without filename");
      }

      node->registers[match[2]] = std::make_unique<FileInRegister>(match[2], &*node, match[4].str());
    }
    else if (match[1] == "file_out")
    {
      if (!match[4].matched)
      {
        throw Error("Tried to create file_out register without filename");
      }

      node->registers[match[2]] = std::make_unique<FileOutRegister>(match[2], &*node, match[4].str());
    }

    std::string lower(match[2]);
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    hwRegMap.emplace(lower, node->registers[match[2]].get());
  }
  else if (std::regex_match(line, match, startStmt))
  {
    finalizeActiveMachine();
    pMachineBeingAssembled = std::make_unique<Machine>();
    pMachineBeingAssembled->name = match[1];
  }
  else if (std::regex_match(line, match, homeStmt))
  {
    auto node = std::find(nodes.begin(), nodes.end(), match[1].str());
    if (node == nodes.end())
    {
      throw Error("Tried to set home to unrecognized node");
    }

    pHomeNode = &*node;
  }
  else
  {
    throw Error("Unrecognized config directive: " + line);
  }
}

void Network::processPreprocessorDirective(const std::string& line)
{
  if (line.find("@rep") == 0)
  {
    repCount = std::stoul(line.data() + 5);
    addRepLines = true;
  }
  else if (line.find("@end") == 0)
  {
    if (!addRepLines)
    {
      throw Error("Found @end without corresponding @rep");
    }

    addRepLines = false;

    static std::regex incrementor(R"r((.*)@\{(-?\d+),(-?\d+)\}(.*))r");

    for (size_t i = 0; i < repCount; i++)
    {
      for (const auto& line : repLines)
      {
        std::smatch match;
        if (std::regex_search(line, match, incrementor))
        {
          Number start = std::stol(match[2]);
          Number inc = std::stol(match[3]);

          std::ostringstream procLine;
          procLine << match[1] << start + inc * i << match[4];

          processInstruction(procLine.str());
        }
        else
        {
          processInstruction(line);
        }
      }
    }

    repLines.clear();
  }
}

void Network::processInstruction(const std::string& line)
{
  static std::regex noArgs(R"((halt|kill|mode|make|drop|wipe|noop|dump))");
  static std::regex singleArg(R"((mark|repl|jump|tjmp|fjmp|test|link|host|void|grab|file|seek|rand|dump)\s+(\S+))");
  static std::regex doubleArg(R"((copy)\s+(\S+)\s+(\S+))");
  static std::regex tripleArg(R"((addi|subi|muli|divi|modi|swiz|test)\s+(\S+)\s+(\S+)\s+(\S+))");

  std::smatch match;

  if (std::regex_match(line, match, noArgs))
  {
    processNoArgs(match[1]);
  }
  else if (std::regex_match(line, match, singleArg))
  {
    processSingleArg(match[1], match[2]);
  }
  else if (std::regex_match(line, match, doubleArg))
  {
    processDoubleArg(match[1], match[2], match[3]);
  }
  else if (std::regex_match(line, match, tripleArg))
  {
    processTripleArg(match[1], match[2], match[3], match[4]);
  }
  else
  {
    throw Error("Unrecognized or invalid instruction: " + line);
  }
}

void Network::processNoArgs(const std::string& mne)
{
  static std::map<std::string, Instruction::Opcode> opcodeMap = {
    {"halt", Instruction::Opcode::Halt},
    {"kill", Instruction::Opcode::Kill},
    {"mode", Instruction::Opcode::Mode},
    {"make", Instruction::Opcode::Make},
    {"drop", Instruction::Opcode::Drop},
    {"wipe", Instruction::Opcode::Wipe},
    {"noop", Instruction::Opcode::Noop},
    {"dump", Instruction::Opcode::Dump0}
  };

  auto iter = opcodeMap.find(mne);
  if (iter == opcodeMap.end())
  {
    throw Error("Unrecognized mnemonic: " + mne);
  }

  pMachineBeingAssembled->code.emplace_back(iter->second);
}

void Network::processSingleArg(const std::string& mne, const std::string& op1)
{
  if (mne == "mark")
  {
    addressLookup.emplace(op1, pMachineBeingAssembled->code.size());
  }
  else if (mne == "repl")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Repl, op1);
  }
  else if (mne == "jump")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Jump, op1);
  }
  else if (mne == "tjmp")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Tjmp, op1);
  }
  else if (mne == "fjmp")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Fjmp, op1);
  }
  else if (mne == "test")
  {
    if (op1 == "mrd")
    {
      pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Test1, Instruction::Register::M);
    }
    else if (op1 == "eof")
    {
      pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Test1, Instruction::Register::F);
    }
  }
  else if (mne == "link")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Link, regOrVal(op1));
  }
  else if (mne == "host")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Host, reg(op1));
  }
  else if (mne == "void")
  {
    if (op1 == "m")
    {
      pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Void, Instruction::Register::M);
    }
    else if (op1 == "f")
    {
      pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Void, Instruction::Register::F);
    }
    else
    {
      throw Error("Void only accepts M or F");
    }
  }
  else if (mne == "grab")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Grab, regOrVal(op1));
  }
  else if (mne == "file")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::File, reg(op1));
  }
  else if (mne == "seek")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Seek, regOrVal(op1));
  }
  else if (mne == "rand")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Rand, reg(op1));
  }
  else if (mne == "dump")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Dump1, op1);
  }
  else
  {
    throw Error("Unrecognized mnemonic: " + mne);
  }
}

void Network::processDoubleArg(const std::string& mne, const std::string& op1, const std::string& op2)
{
  if (mne == "copy")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Copy, regOrVal(op1), reg(op2));
  }
  else
  {
    throw Error("Unrecognized mnemonic: " + mne);
  }

  int numM = 0;

  numM += (std::holds_alternative<Instruction::Register>(pMachineBeingAssembled->code.back().op1) && std::get<Instruction::Register>(pMachineBeingAssembled->code.back().op1) == Instruction::Register::M) ? 1 : 0;
  numM += (std::holds_alternative<Instruction::Register>(pMachineBeingAssembled->code.back().op2) && std::get<Instruction::Register>(pMachineBeingAssembled->code.back().op2) == Instruction::Register::M) ? 1 : 0;

  if (numM > 1)
  {
    throw Error("Referenced M register too many times in one instruction");
  }
}

void Network::processTripleArg(const std::string& mne, const std::string& op1, const std::string& op2, const std::string& op3)
{
  // addi|subi|muli|divi|modi|swiz|test

  if (mne == "addi")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Addi, regOrVal(op1), regOrVal(op2), reg(op3));
  }
  else if (mne == "subi")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Subi, regOrVal(op1), regOrVal(op2), reg(op3));
  }
  else if (mne == "muli")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Muli, regOrVal(op1), regOrVal(op2), reg(op3));
  }
  else if (mne == "divi")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Divi, regOrVal(op1), regOrVal(op2), reg(op3));
  }
  else if (mne == "modi")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Modi, regOrVal(op1), regOrVal(op2), reg(op3));
  }
  else if (mne == "swiz")
  {
    pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::Swiz, regOrVal(op1), regOrVal(op2), reg(op3));
  }
  else if (mne == "test")
  {
    if (op2 == "<")
    {
      pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::TestLt, regOrVal(op1), regOrVal(op3));
    }
    else if (op2 == "=")
    {
      pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::TestEq, regOrVal(op1), regOrVal(op3));
    }
    else if (op2 == ">")
    {
      pMachineBeingAssembled->code.emplace_back(Instruction::Opcode::TestGt, regOrVal(op1), regOrVal(op3));
    }
  }
  else
  {
    throw Error("Unrecognized mnemonic: " + mne);
  }

  int numM = 0;

  numM += (std::holds_alternative<Instruction::Register>(pMachineBeingAssembled->code.back().op1) && std::get<Instruction::Register>(pMachineBeingAssembled->code.back().op1) == Instruction::Register::M) ? 1 : 0;
  numM += (std::holds_alternative<Instruction::Register>(pMachineBeingAssembled->code.back().op2) && std::get<Instruction::Register>(pMachineBeingAssembled->code.back().op2) == Instruction::Register::M) ? 1 : 0;
  numM += (std::holds_alternative<Instruction::Register>(pMachineBeingAssembled->code.back().op3) && std::get<Instruction::Register>(pMachineBeingAssembled->code.back().op3) == Instruction::Register::M) ? 1 : 0;

  if (numM > 1)
  {
    throw Error("Referenced M register too many times in one instruction");
  }
}

void Network::finalizeActiveMachine()
{
  if (pMachineBeingAssembled)
  {
    if (addRepLines)
    {
      throw Error("Missing @end after @rep");
    }

    if (!pHomeNode)
    {
      throw Error("Tried to finalize machine before home node was set");
    }

    if (pHomeNode->machines.size() + pHomeNode->files.size() < pHomeNode->capacity)
    {
      size_t curAddr = 0;

      for (auto& rInst : pMachineBeingAssembled->code)
      {
        if (rInst.opcode == Instruction::Opcode::Jump ||
          rInst.opcode == Instruction::Opcode::Tjmp ||
          rInst.opcode == Instruction::Opcode::Fjmp ||
          rInst.opcode == Instruction::Opcode::Repl)
        {
          std::string label = std::get<std::string>(rInst.op1);
          auto iter = addressLookup.find(label);
          if (iter == addressLookup.end())
          {
            throw Error("Tried to jump/repl to unrecognized label: " + label);
          }

          rInst.op1 = iter->second;
        }
      }

      stats.size += pMachineBeingAssembled->code.size();
      pHomeNode->machines.emplace_back(std::move(pMachineBeingAssembled));
    }
    else
    {
      throw Error("Tried to add machine to node, but node is already full");
    }

    addressLookup.clear();
  }
}

Instruction::Operand Network::regOrVal(const std::string& op)
{
  if (op == "x")
  {
    return Instruction::Register::X;
  }

  if (op == "t")
  {
    return Instruction::Register::T;
  }

  if (op == "m")
  {
    return Instruction::Register::M;
  }

  if (op == "f")
  {
    return Instruction::Register::F;
  }

  auto iter = hwRegMap.find(op);
  if (iter != hwRegMap.end())
  {
    return iter->second;
  }

  Number val = std::stol(op);
  return val;
}

Instruction::Operand Network::reg(const std::string& op)
{
  if (op == "x")
  {
    return Instruction::Register::X;
  }

  if (op == "t")
  {
    return Instruction::Register::T;
  }

  if (op == "m")
  {
    return Instruction::Register::M;
  }

  if (op == "f")
  {
    return Instruction::Register::F;
  }

  auto iter = hwRegMap.find(op);
  if (iter != hwRegMap.end())
  {
    return iter->second;
  }

  throw Error("Unrecognized register: " + op);
}

std::optional<Value> Network::get(Node& rNode, Machine& rMachine, const Instruction::Operand& src)
{
  std::optional<Value> ret;

  std::visit([&](const auto& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, std::monostate>)
      {
        throw Error("Tried to use uninitialized operand");
      }
      else if constexpr (std::is_same_v<T, Instruction::Register>)
      {
        switch (std::get<Instruction::Register>(src))
        {
          case Instruction::Register::X:
            ret = rMachine.x;
            break;
          case Instruction::Register::T:
            ret = rMachine.t;
            break;
          case Instruction::Register::M:
          {
            if (rMachine.globalMode && globalChannel.available())
            {
              ret = globalChannel.receive();
            }
            else if (!rMachine.globalMode && rNode.localChannel.available())
            {
              ret = rNode.localChannel.receive();
            }

            break;
          }
          case Instruction::Register::F:
          {
            if (rMachine.file.has_value())
            {
              ret = rMachine.file->read();
            }
            else
            {
              throw MachineFailure("Tried to read from file, but no file held");
            }

            break;
          }
        }
      }
      else if constexpr (std::is_same_v<T, Number>)
      {
        ret = arg;
      }
      else if constexpr (std::is_same_v<T, Instruction::Address>)
      {
        throw Error("Tried to read address as value");
      }
      else if constexpr (std::is_same_v<T, HwRegister*>)
      {
        if (arg->pHost == &rNode)
        {
          ret = arg->read();
        }
        else
        {
          throw MachineFailure("Tried to read inaccessible hardware register");
        }
      }
    }, src);

  if (ret)
  {
    ret = clamp(*ret);
  }

  return ret;
}

bool Network::set(Node& rNode, Machine& rMachine, const Instruction::Operand& dest, const Value& val)
{
  Value clamped = clamp(val);
  bool ret = true;

  std::visit([&](const auto& arg)
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, std::monostate>)
      {
        throw Error("Tried to use uninitialized operand");
      }
      else if constexpr (std::is_same_v<T, Instruction::Register>)
      {
        switch (std::get<Instruction::Register>(dest))
        {
          case Instruction::Register::X:
            rMachine.x = clamped;
            break;
          case Instruction::Register::T:
            rMachine.t = clamped;
            break;
          case Instruction::Register::M:
          {
            if (rMachine.globalMode)
            {
              if (globalChannel.available())
              {
                ret = false;
              }
              else
              {
                globalChannel.send(clamped);
              }
            }
            else
            {
              if (rNode.localChannel.available())
              {
                ret = false;
              }
              else
              {
                rNode.localChannel.send(clamped);
              }
            }

            break;
          }
          case Instruction::Register::F:
          {
            if (rMachine.file.has_value())
            {
              rMachine.file->write(clamped);
            }
            else
            {
              throw MachineFailure("Tried to write to file, but no file held");
            }

            break;
          }
        }
      }
      else if constexpr (std::is_same_v<T, Number>)
      {
        throw Error("Tried to write to literal");
      }
      else if constexpr (std::is_same_v<T, Instruction::Address>)
      {
        throw Error("Tried to write to code address");
      }
      else if constexpr (std::is_same_v<T, HwRegister*>)
      {
        if (arg->pHost == &rNode)
        {
          arg->write(clamped);
        }
        else
        {
          throw MachineFailure("Tried to write to inaccessible hardware register");
        }
      }
    }, dest);

  return ret;
}

Value Network::clamp(const Value& val)
{
  if (std::holds_alternative<Number>(val))
  {
    return std::clamp(std::get<Number>(val), rangeMin, rangeMax);
  }
  else
  {
    return val;
  }
}

Value Network::swiz(const Value& input, const Value& mask)
{
  if (!std::holds_alternative<Number>(input))
  {
    throw MachineFailure("Tried to swiz a string");
  }

  if (!std::holds_alternative<Number>(mask))
  {
    throw MachineFailure("Tried to use a string to swiz a number");
  }

  Number output = 0;
  std::string inStr = std::to_string(std::get<Number>(input));
  std::string maskStr = std::to_string(std::get<Number>(mask));

  bool negative = false;

  for (signed char c : maskStr)
  {
    if (c == '-')
    {
      negative = true;
      continue;
    }

    output *= 10;

    if (c == '0')
    {
      continue;
    }

    size_t dgt = c - '1';

    if (dgt < inStr.size())
    {
      output += inStr[inStr.size() - dgt - 1] - '0';
    }
  }

  if (negative)
  {
    output *= -1;
  }
  
  return output;
}
} // namespace ep
