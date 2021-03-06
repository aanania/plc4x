<#--
  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.
-->
${helper.getIncludesDirectory()?replace(".", "/")}/${helper.camelCaseToSnakeCase(typeName)}.h
/*
  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.
*/

#ifndef ${helper.getCTypeName(type.name)?upper_case}_H_
#define ${helper.getCTypeName(type.name)?upper_case}_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
<#--
    Add any import statements for partent-types, complex types used in properties or parser arguments.
-->
<#if helper.getComplexTypeReferences()?has_content>
    <#list helper.getComplexTypeReferences() as typeReference>
#include "${helper.camelCaseToSnakeCase(typeReference)}.h"
    </#list>
</#if>

<#--
    First output the enum itself together with it's constants
-->
enum ${helper.getCTypeName(type.name)} {
<#list type.enumValues as enumValue>
  ${helper.getCTypeName(type.name)}_${enumValue.name} = ${enumValue.value}<#sep>,
</#sep></#list>

};
typedef enum ${helper.getCTypeName(type.name)} ${helper.getCTypeName(type.name)};

// Create an empty NULL-struct
static const ${helper.getCTypeName(type.name)} ${helper.getCTypeName(type.name)}_null;

<#--
    If this enum also defines constant values, output one function for each.
    Each of these functions consist of a simple switch statement using the enum as switch-value
    returning the constant for a given enum value.
-->
<#if type.constantNames?has_content>
    <#list type.constantNames as constantName>

${helper.getLanguageTypeNameForTypeReference(type.getConstantType(constantName))} ${helper.getCTypeName(type.name)}_get_${helper.camelCaseToSnakeCase(constantName)}(${helper.getCTypeName(type.name)} value) {
  switch(value) {
        <#list helper.getUniqueEnumValues(type.enumValues) as enumValue>
    case ${helper.escapeValue(type.type, enumValue.value)}: { /* '${enumValue.value}' */
      return ${helper.escapeEnumValue(type.getConstantType(constantName), enumValue.getConstant(constantName))?no_esc};
    }<#sep>
    </#sep></#list>

    default: {
      return 0;
    }
  }
}
    </#list>
</#if>

#ifdef __cplusplus
}
#endif
#endif  // ${helper.getCTypeName(type.name)?upper_case}_H_
