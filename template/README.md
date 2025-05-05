# CGen Templates

This directory contains templates used by CGen to generate project files.

## Template Organization

Templates are organized into the following categories:

1. **Core** - Essential templates for any project type
2. **Project Types** - Type-specific templates (binary, library, header-only)
3. **Package Managers** - Templates for different package managers

For a detailed overview of the template structure, see [structure.md](structure.md).

## Placeholder System

Templates use placeholders that are replaced with actual values during generation.
All placeholders follow the format `{PLACEHOLDER_NAME}`.

### Common Placeholders

- `{PROJECT_NAME}` - Project name from config
- `{PROJECT_VERSION}` - Project version from config
- `{PROJECT_DESCRIPTION}` - Project description from config
- `{PROJECT_NAMESPACE}` - Project namespace from config
- `{CPP_STANDARD}` - C++ standard from config
- `{DEPENDENCIES}` - Project dependencies
- `{CMAKE_OPTIONS}` - CMake options
- `{CMAKE_DEFINES}` - CMake defines

### Special Placeholders

Some templates have special placeholders specific to their context:

- Module templates: `{MODULE_NAME}`, `{NAMESPACE}`
- Package manager templates: `{CPM_PACKAGES}`, `{FIND_PACKAGES}`

## Adding New Templates

To add a new template:

1. Place the template file in the appropriate category directory
2. Use the `.template` file extension
3. Use the standard `{PLACEHOLDER}` format for placeholders
4. Document any special placeholders specific to your template

## Template Customization

Users can override any template by placing a custom version in their project's
`.cgen/templates/` directory with the same relative path.