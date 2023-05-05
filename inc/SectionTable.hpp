#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Constants.hpp"

using namespace std;

struct SectionDefinition
{
    uint32_t base = 0;
    uint32_t length = 0;
    string name;

    friend ostream &operator<<(ostream &os, const SectionDefinition &sd);
};

class SectionTable
{
  public:
    SectionTable();

    int insertSectionDefinition(SectionDefinition &);

    void closeLastSection(uint32_t endLocation);

    friend ostream &operator<<(ostream &os, const SectionTable &st);

  private:
    unordered_map<string, uint32_t> sectionMappings;
    vector<SectionDefinition> sectionDefinitions;

    uint32_t currentSection = UND;
};