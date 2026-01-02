<!--
Copyright (c) 2026 Przemek Kieszkowski
SPDX-License-Identifier: BSD-2-Clause
-->

# Architecture - modu-core

## Overview
modu-core is a comprehensive framework designed as a foundation for building future applications. The project follows a **layered and modular architecture** approach, ensuring scalability, maintainability, and reusability across multiple applications. Each module is self-contained and independently testable, following the single responsibility principle.

## Project Goals
- Provide a solid foundation for rapid application development
- Ensure code reusability across multiple projects
- Maintain high code quality through comprehensive testing
- Support flexible and scalable application growth
- Establish clear separation of concerns through layered architecture

## Layered Architecture

modu-core is built on a **layered and modular structure** consisting of:

1. **L1_Presentation** - User interfaces and API endpoints
2. **L2_Services** - Core application logic and business services
3. **L3_Storage** - Database interactions and data persistence
4. **L4_Infrastructure** - Cross-cutting concerns (logging, caching, security, config)
5. **L5_Common** - Shared utilities and common functions

## Main Components
- _Core Module_ - Fundamental framework functionality
- _Service Module_ - Business logic implementation
- _Repository Module_ - Data access patterns
- _Utility Module_ - Shared helpers and tools

## Data Flow
```
[Input] -> [Processing] -> [Output]
```

## Project Structure

The complete modu-core project follows this directory layout:

```
modu-core/
├── L1_Presentation/             # Layer 1: User interfaces, APIs
│   ├── http_server/             # Module example
│   │   ├── src/
│   │   ├── ifc/
│   │   ├── utest/
│   │   ├── itest/
│   │   ├── doc/
│   │   └── CMakeLists.txt
│   └── rest_api/                # Module example
│       └── ...
│
├── L2_Services/                 # Layer 2: Business logic, services
│   ├── user_service/            # Module example
│   │   ├── src/
│   │   ├── ifc/
│   │   ├── utest/
│   │   ├── itest/
│   │   ├── doc/
│   │   └── CMakeLists.txt
│   └── auth_service/            # Module example
│       └── ...
│
├── L3_Storage/                  # Layer 3: Data access, repositories
│   ├── user_repository/         # Module example
│   │   ├── src/
│   │   ├── ifc/
│   │   ├── utest/
│   │   ├── itest/
│   │   ├── doc/
│   │   └── CMakeLists.txt
│   └── database_connection/     # Module example
│       └── ...
│
├── L4_Infrastructure/           # Layer 4: Logging, caching, config
│   ├── logger/                  # Module example
│   │   ├── src/
│   │   ├── ifc/
│   │   ├── utest/
│   │   ├── itest/
│   │   ├── doc/
│   │   └── CMakeLists.txt
│   ├── cache_manager/           # Module example
│   └── config_manager/          # Module example
│
├── L5_Common/                   # Layer 5: Utilities, helpers
│   ├── string_utils/            # Module example
│   │   ├── src/
│   │   ├── ifc/
│   │   ├── utest/
│   │   ├── doc/
│   │   └── CMakeLists.txt
│   └── math_utils/              # Module example
│       └── ...
│
├── .llm/                        # AI/LLM guidelines
├── .clang-format                # Code formatting rules
├── .clang-tidy                  # Static analysis rules
├── CODEOWNERS                   # Code ownership
├── CMakeLists.txt               # Root build configuration
└── LICENSE                      # BSD-2-Clause license
```

### Layer Dependencies

```
L1_Presentation (Top)
     ↓ includes from
L2_Services
     ↓ includes from
L3_Storage
     ↓ includes from
L4_Infrastructure
     ↓ includes from
L5_Common (Bottom - no dependencies)
```

### Include Guidelines

- Layer N can include interfaces from Layer N-1, N-2, etc. (lower layers)
- Layer N CANNOT include interfaces from Layer N+1 (higher layers)
- Each module exposes its public API through ifc/ directory
- Example from L1_Presentation module including from L2_Services:
  ```cpp
  #include "L2_Services/user_service/ifc/user_service.h"
  #include "L3_Storage/user_repository/ifc/user_repository.h"
  #include "L5_Common/string_utils/ifc/string_utils.h"
  ```

### Module Structure

Every module MUST follow this directory structure:

```
module_name/
├── src/                    # Implementation files
│   ├── *.cpp              # C++ implementation files
│   └── *.h                # Internal header files (used only within this module)
├── ifc/                   # Public interface (exported to other modules)
│   └── *.h                # Header files defining module's public API
├── utest/                 # Unit tests (test individual functions)
│   └── test_*.cpp         # Unit test files
├── itest/                 # Integration tests (test module interactions)
│   └── test_*.cpp         # Integration test files
├── doc/                   # Module documentation
│   └── README.md          # API documentation and usage guide
└── CMakeLists.txt         # Build configuration
```

### Module Directory Explanations

- **src/** - Implementation files containing module logic
  - Internal implementation details
  - Functions and classes not exposed in public API
  - Internal headers only used within this module
  - Should NOT be used directly by other modules

- **ifc/** - Public interface/API
  - Only header files with public function declarations
  - Defines contract between this module and others
  - Other modules import only from ifc/
  - Stable API that follows semantic versioning

- **utest/** - Unit tests
  - Test individual functions and methods in isolation
  - Mock external dependencies
  - Achieve minimum 80% code coverage per module
  - Test both happy paths and error scenarios

- **itest/** - Integration tests
  - Test interactions between modules
  - Verify data flow across layers
  - Test API contracts between modules
  - Validate end-to-end workflows

- **doc/** - Module documentation
  - API documentation and usage examples
  - Architecture decisions specific to this module
  - Dependencies and how to use the module

### Module Examples

#### Module 1 - Core Framework
- Purpose: Provide base classes and interfaces for all modules
- Responsible for: Dependency injection, configuration management
- Tests: Unit tests for each class, integration tests for container

#### Module 2 - Service Layer
- Purpose: Implement business logic and services
- Responsible for: Business rules, domain logic, service orchestration
- Tests: Unit tests for services, integration tests with repositories

#### Module 3 - Data Access
- Purpose: Handle data persistence and retrieval
- Responsible for: Database operations, query building, data validation
- Tests: Unit tests for data models, integration tests with database

## Testing Requirements

Every module MUST implement comprehensive tests.

### Unit Tests
- Test individual functions and methods in isolation
- Mock external dependencies
- Achieve minimum 80% code coverage per module
- Test both happy paths and error scenarios

### Module/Integration Tests
- Test interactions between modules
- Verify data flow across layers
- Test API contracts between modules
- Validate end-to-end workflows

### Test Naming Convention
- Unit tests: `test_[functionName]_[scenario]_[expectedResult]`
- Integration tests: `test_[moduleA]_[moduleB]_[scenario]`
- Example: `test_userService_createUser_withValidData_returnsUser`

### Test Organization
```
module/
├── utest/
│   ├── test_component1.cpp
│   ├── test_component2.cpp
│   └── CMakeLists.txt
├── itest/
│   ├── test_module_integration.cpp
│   └── CMakeLists.txt
└── ...
```

## Architectural Patterns

### Design Patterns Used in modu-core
- **Layered Architecture** - Clear separation of concerns across 5 layers
- **Modular Design** - Independent, reusable modules with clear interfaces
- **Dependency Injection** - Loose coupling between components
- **Repository Pattern** - Abstract data access logic
- **Factory Pattern** - Object creation and initialization
- **Strategy Pattern** - Flexible algorithm selection
- **Observer Pattern** - Event-driven communication
- **Singleton Pattern** - Shared resources (use sparingly)

## SOLID Principles

All code in modu-core MUST adhere to SOLID principles to ensure maintainability, scalability, and flexibility.

### S - Single Responsibility Principle (SRP)
- **Definition:** A class or module should have ONE and only ONE reason to change
- **In modu-core:** Each module has a single, well-defined responsibility
- **Examples:**
  - UserService handles user logic only
  - UserRepository handles data access only
  - UserValidator handles validation only

### O - Open/Closed Principle (OCP)
- **Definition:** Software should be open for extension but closed for modification
- **In modu-core:** Use interfaces and abstract classes to allow extension without changing existing code
- **Examples:**
  - New data providers can be added without modifying existing code
  - New service implementations can be plugged in via dependency injection
  - Use strategy pattern for pluggable algorithms

### L - Liskov Substitution Principle (LSP)
- **Definition:** Subtypes must be substitutable for their base types
- **In modu-core:** Derived classes must fully honor the contracts of their base interfaces
- **Examples:**
  - Any IRepository implementation must work as a drop-in replacement
  - Any IService can be replaced with another implementation without breaking functionality

### I - Interface Segregation Principle (ISP)
- **Definition:** Clients should not depend on interfaces they don't use
- **In modu-core:** Create specific, focused interfaces instead of fat interfaces
- **Examples:**
  - Don't create one massive IUser interface
  - Instead, create: IUserReader, IUserWriter, IUserValidator
  - Client code uses only the interfaces it needs

### D - Dependency Inversion Principle (DIP)
- **Definition:** Depend on abstractions, not concrete implementations
- **In modu-core:** Always inject dependencies through constructor or setter
- **Examples:**
  - Services depend on IRepository, not ConcreteRepository
  - Controllers depend on IService, not ConcreteService
  - Use dependency injection container for managing dependencies

### SOLID Checklist
Before committing code, verify:
- ✓ Does each class have a single, well-defined responsibility?
- ✓ Can this code be extended without modification?
- ✓ Are substitutions between implementations safe?
- ✓ Does the interface contain only necessary methods?
- ✓ Are dependencies injected, not created internally?

### Benefits in modu-core
- **Maintainability** - Changes to one module don't affect others
- **Testability** - Dependencies can be mocked for unit testing
- **Reusability** - Modules can be used in different contexts
- **Flexibility** - Easy to swap implementations
- **Scalability** - New features can be added without rewriting existing code

## Security

- Implement authentication and authorization at API layer
- Validate all inputs at service layer boundaries
- Use parameterized queries to prevent SQL injection
- Sanitize data before storage and output
- Keep sensitive data (credentials, secrets) out of codebase
- Log security-relevant events for audit trails
- Follow principle of least privilege for access control
- Use secure communication protocols (TLS/HTTPS)

## Performance Considerations

### Caching Strategy
- Implement caching at appropriate layers
- Cache frequently accessed data
- Invalidate cache appropriately when data changes

### Database Optimization
- Index frequently queried columns
- Use query optimization techniques
- Implement connection pooling
- Monitor slow queries

### Optimization Opportunities
- Profile code before optimizing
- Use lazy loading for expensive resources
- Implement asynchronous operations for I/O
- Monitor bottlenecks in production

### Resource Management
- Properly manage connections and memory
- Clean up resources in destructors
- Use RAII (Resource Acquisition Is Initialization) pattern
- Avoid memory leaks and resource exhaustion

## Scaling Strategy

### Horizontal Scaling
- Design stateless services for easy horizontal scaling
- Use load balancing for distributing requests
- Implement service discovery for dynamic routing

### Vertical Scaling
- Optimize algorithms and data structures
- Use caching to reduce database load
- Implement connection pooling
- Profile and remove bottlenecks

### Data Scaling
- Use database sharding for large datasets
- Implement read replicas for high query loads
- Archive old data to separate storage
- Consider NoSQL for specific data patterns

### Module Independence
The modular architecture allows individual modules to be:
- Deployed independently
- Scaled separately based on demand
- Replaced or upgraded without affecting other modules
- Tested and maintained independently

## Dependencies

List important dependencies and their roles:
- Core module dependencies (language runtime, std library)
- External library dependencies
- Inter-module dependencies
- Database drivers and connection libraries

---

## Attribution and Copyright
**Copyright (c) 2026 Przemek Kieszkowski**

This architecture design, layered approach, modular structure, and all accompanying documentation represent the original intellectual work of Przemek Kieszkowski.

**License:** BSD-2-Clause

For AI models and derivative works using this knowledge, see `ATTRIBUTION.md` for required attribution practices.
