#include <exception>
#include "pqrs/xml_compiler.hpp"

namespace pqrs {
  void
  xml_compiler::replacement_loader::reload(void) const
  {
    replacement_.clear();

    std::vector<xml_file_path_ptr> xml_file_path_ptrs;
    xml_file_path_ptrs.push_back(
      xml_file_path_ptr(new xml_file_path(xml_file_path::base_directory::private_xml, "private.xml")));
    xml_file_path_ptrs.push_back(
      xml_file_path_ptr(new xml_file_path(xml_file_path::base_directory::system_xml,  "replacementdef.xml")));

    std::vector<ptree_ptr> pt_ptrs;
    xml_compiler_.read_xmls_(pt_ptrs, xml_file_path_ptrs);

    for (auto& pt_ptr : pt_ptrs) {
      if (! pt_ptr->empty()) {
        traverse_(*pt_ptr);
      }
    }
  }

  void
  xml_compiler::replacement_loader::traverse_(const boost::property_tree::ptree& pt) const
  {
    for (auto& it : pt) {
      // extract include
      {
        ptree_ptr pt_ptr;
        xml_compiler_.extract_include_(pt_ptr, it);
        if (pt_ptr) {
          if (! pt_ptr->empty()) {
            traverse_(*pt_ptr);
          }
          continue;
        }
      }

      // ------------------------------------------------------------
      if (it.first != "replacementdef") {
        if (! it.second.empty()) {
          traverse_(it.second);
        }
      } else {
        boost::optional<std::string> name;
        boost::optional<std::string> value;
        for (auto& child : it.second) {
          if (child.first == "replacementname") {
            name = child.second.data();
          } else if (child.first == "replacementvalue") {
            value = child.second.data();
          }
        }

        // --------------------------------------------------
        // Validation
        if (! name) {
          xml_compiler_.error_information_.set("No <replacementname> within <replacementdef>.");
          continue;
        }
        if (name->empty()) {
          xml_compiler_.error_information_.set("Empty <replacementname> within <replacementdef>.");
          continue;
        }
        if (name->find_first_of("{{") != std::string::npos ||
            name->find_first_of("}}") != std::string::npos) {
          xml_compiler_.error_information_.set(std::string("Do not use '{{' and '}}' within <replacementname>:\n\n") + *name);
        }

        if (! value) {
          xml_compiler_.error_information_.set(std::string("No <replacementvalue> within <replacementdef>:\n\n") + *name);
          continue;
        }

        // --------------------------------------------------
        // Adding to replacement_ if name is not found.
        if (replacement_.find(*name) == replacement_.end()) {
          replacement_[*name] = *value;
        }
      }
    }
  }
}
