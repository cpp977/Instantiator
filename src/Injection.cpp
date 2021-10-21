#include "Injection.hpp"

#include <ostream>
#include <sstream>

std::string Injection::getInstantiation() const
{
    std::stringstream res;
    res << "\ntemplate ";
    if(not is_constructor) { res << return_type << " "; }
    if(is_member) {
        res << class_name;
        if(class_Ttypes.size() > 0) {
            res << "<";
            for(std::size_t i = 0; i < class_Ttypes.size(); i++) {
                if(i + 1 < class_Ttypes.size()) {
                    res << class_Ttypes[i] << ",";
                } else {
                    res << class_Ttypes[i] << ">";
                }
            }
        }
        res << "::";
    }
    res << func_name;
    if(func_Ttypes.size() > 0) {
        res << "<";
        for(std::size_t i = 0; i < func_Ttypes.size(); i++) {
            if(i + 1 < func_Ttypes.size()) {
                res << func_Ttypes[i] << ",";
            } else {
                res << func_Ttypes[i] << ">";
            }
        }
    }
    res << "(";
    for(std::size_t i = 0; i < params.size(); i++) {
        if(i + 1 < params.size()) {
            res << params_name[i] << ",";
        } else {
            res << params_name[i];
        }
    }
    res << ")";
    if(is_member) {
        if(is_const) { res << " const"; }
    }
    res << ";";
    return res.str();
}

std::ostream& operator<<(std::ostream& stream, const Injection& toDo)
{
    if(toDo.is_member) {
        if(toDo.is_const) { stream << "const "; }
        stream << "Member ";
    } else {
        stream << "Free ";
    }
    stream << "function: " << toDo.func_name;
    if(toDo.func_Ttypes.size() > 0) {
        stream << " (with template params: ";
        for(const auto& p : toDo.func_Ttypes) { stream << p << " "; }
        stream << ")";
    }
    if(toDo.is_member) {
        stream << " of class " << toDo.class_name;
        if(toDo.class_Ttypes.size() > 0) {
            stream << " (with template params: ";
            for(const auto& p : toDo.class_Ttypes) { stream << p << " "; }
            stream << ")";
        }
    }
    stream << " with params: ";
    for(const auto& p : toDo.params_name) { stream << p << " "; }
    stream << "; with return type " << toDo.return_type;
    return stream;
}
