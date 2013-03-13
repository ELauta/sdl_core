"""SmartSchema generator.

Generator for SmartObjects schema source code.

"""

import os
import string

from generator import Model

class GenerateError(Exception):

    """Generate error.

    This exception is raised when SmartSchema generator is unable to create output from given model.

    """

    pass

class SmartSchema(object):
    
    """SmartSchema generator."""
    
    def __init__(self):
        """Constructor"""
        self._enums_content = ""
        
    def generate(self, model, filename, namespace, destination_dir):
        """Generate SmartObject source files.

        Generates source code files at destination directory in 
        accordance with given model in specified namespace.
        
        Keyword arguments:
        model -- model to generate source code for.
        filename -- name of initial XML file.
        namespace -- name of destination namespace.
        destination_dir -- directory to create source files. 

        """
                
        if model is None:
            raise GenerateError("Given model is None")

        os.makedirs(destination_dir)

        cpp_file_content = ""
        hpp_file_content = ""

        cpp_file = open(os.path.join(destination_dir, filename.join(".cpp")), "w")
        cpp_file.write(cpp_file_content)
        cpp_file.close()

        hpp_file = open(os.path.join(destination_dir, filename.join(".hpp")) , "w")   
        hpp_file.wirte(hpp_file_content)
        hpp_file.close()

    def _generate_enums(self, enums):
        if enums is None:
            raise GenerateError("Enums is None")

        for enum in enums:
            self._enums_content = self._enums_content.join(self._generate_enum(enum))
            
    def _generate_enum(self, enum):
        return self._comment_template.substitute(comment = self._generate_comment(enum),
                                                 name = enum.name,
                                                 enum_items = self._generate_enum_elements(enum))

    def _generate_enum_elements(self, enum):
        pass

    def _generate_enum_element(self, enum_element):
        return self._enum_element_template.substitute(comment = self._generate_comment(enum_element),
                                                      name = enum_element.internal_name if enum_element.internal_name is not None else enum_element.name,
                                                      value = enum_element.value)

    def _generate_comment(self, interface_item_base):
        brief_type_title = None
        interface_item_base_classname = interface_item_base.__class__.__name__ 
        if interface_item_base_classname in self._model_types_briefs:
            brief_type_title = self._model_types_briefs[interface_item_base_classname]
        else:
            raise GenerateError("Unable to create comment for unknown type " +
                                interface_item_base_classname)
        
        name = interface_item_base.internal_name if type(interface_item_base) is Model.EnumElement and interface_item_base.internal_name is not None else interface_item_base.name
        brief_description = " * @brief {0}{1}.".format(brief_type_title, name)
        
        if  interface_item_base.description or \
            interface_item_base.design_description or \
            interface_item_base.issues  or \
            interface_item_base.todos:
            brief_description = "".join([brief_description, "\n *\n"])
        else:
            brief_description = "".join([brief_description, "\n"])
        
        description = "".join(map(lambda x: " * {0}\n".format(x),
                                  interface_item_base.description))
        if description is not "":
            description = "".join([description, " *\n"])
        
        design_description = "".join(map(lambda x: " * {0}\n".format(x),
                                         interface_item_base.design_description))
        if design_description is not "":
            design_description = "".join([design_description, " *\n"])
        
        issues = "".join(map(lambda x: " * @note {0}\n".format(x),
                             interface_item_base.issues))
        if issues is not "":
            issues = "".join([issues, " *\n"])
        
        todos = "".join(map(lambda x: " * @todo {0}\n".format(x),
                            interface_item_base.todos))

        return self._comment_template.substitute(brief_description = brief_description,
                                                 description = description,
                                                 design_description = design_description,
                                                 issues = issues,
                                                 todos = todos)
        
    _model_types_briefs = dict({"EnumElement" : "", 
                                "Enum" : "Enumeration "})
                
    _comment_template = string.Template(
"""/**
$brief_description$description$design_description$issues$todos */""")
    
    _enum_template = string.Template(
"""$comment
enum $name
{
    $enum_items
}
""")
    
    _enum_element_template = string.Template(
"""$comment
$name = $value""")
    