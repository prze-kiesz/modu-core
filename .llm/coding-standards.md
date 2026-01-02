<!--
Copyright (c) 2026 Przemek Kieszkowski
SPDX-License-Identifier: BSD-2-Clause
-->

# Coding Standards - modu-core
*Aligned with Google Style Guide*

## General Principles
- Code should be clean, readable, and maintainable
- Consistency across the entire project is most important
- Avoid magic numbers - use named constants
- Maximum line length: **80 characters** (hard limit)
- Every file should end with a newline
- Use 2 spaces for indentation (consistently throughout the project)

## Naming Conventions (Google Style)

### Variables
- Format: `camelCase`
- Descriptive and complete words
- Examples: `userCount`, `isActive`, `maxRetries`

### Functions/Methods
- Format: `camelCase`
- Verbs for actions: `getUserData()`, `calculateTotal()`, `setConfig()`
- Questions for booleans: `isValid()`, `hasPermission()`, `canDelete()`

### Classes/Interfaces
- Format: `PascalCase`
- Nouns for classes: `UserManager`, `DataProcessor`, `HttpClient`

### Constants
- Format: `CONSTANT_CASE`
- Used for values that don't change
- Examples: `MAX_ATTEMPTS`, `DEFAULT_TIMEOUT`, `ERROR_CODES`

### Private Members
- Prefix with underscore: `_privateField`, `_internalMethod()`
- Or use suffix: `private_`, `internal_`

## File Structure

### Code Organization
1. License/Copyright (if required)
2. Imports/Includes
3. Constants
4. Types/Interfaces
5. Public functions/classes
6. Private functions/classes

## Comments

### Block Comments
```
// Explain why, not what
// We changed the algorithm to a more efficient one due to performance requirements
```

### Inline Comments (sparingly)
```
// TODO: refactor this function
// FIXME: known bug in Safari browser
// HACK: temporary workaround for issue XYZ
```

### Function Documentation
```
/**
 * Fetches a user from the database
 *
 * @param {string} userId - The user ID
 * @returns {Promise<User>} User data
 * @throws {UserNotFoundError} If user does not exist
 */
async function getUser(userId) { ... }
```

### Comment Guidelines
- Comment WHY, not WHAT
- Avoid obvious comments
- Keep comments up to date with code changes
- One comment per line or block

## Code Formatting

### Functions
- Maximum 40-60 lines (split into smaller functions)
- Single responsibility principle
- Parameters: maximum 3-4 (use config object if more)

### Conditionals and Loops
```
// Good
if (isValid && hasPermission) {
  processData();
}

// Bad - too complex
if (isValid && hasPermission && !isLocked && (userRole === 'admin' || userRole === 'moderator')) {
  processData();
}
```

### Line Formatting
- No spaces inside parentheses: `func(a, b)` not `func( a, b )`
- Space around operators: `a = b + c`
- No spaces in array indices: `array[0]` not `array[ 0 ]`

## Test Naming
- Format: `testNoun_WhenCondition_ExpectResult`
- Example: `testCalculator_WhenAddingNumbers_ExpectCorrectSum`
- Or: `test_addNumbers_returnsCorrectSum`

## Version Control

### Commit Messages
- Short description (50 characters max)
- Format: `type(scope): description`
- Types: `feat`, `fix`, `docs`, `refactor`, `test`, `style`, `perf`, `chore`

### Example
```
feat(auth): add two-factor authentication

- Implement 2FA verification logic
- Integrate with Google Authenticator
- Add unit tests

Closes #123
```

### Guidelines
- Use imperative mood: "Add feature" not "Added feature"
- Describe what and why, not how

## Code Review

### For Reviewers
- Be constructive and supportive
- Ask questions instead of criticizing
- Explain why, not just what
- Approve good solutions

### For Authors
- Ask for clarifications
- Be open to suggestions
- Don't take it personally
- Thank reviewers for feedback

## Testing

### Principles
- Write tests for new code
- Maintain minimum 80% code coverage
- Tests should be independent and repeatable
- One assertion per test (ideal)
- Test edge cases and error conditions

### Test Naming
- Clear and descriptive: what is being tested and the expected result
- Example: `test_getUserById_withValidId_returnsUser`

## Security

- Never hardcode credentials or secrets
- Validate all inputs
- Use parameterized queries
- Sanitize output data
- Log security-related errors

## Performance

- Avoid premature optimization
- Measure before optimizing
- Document performance bottlenecks
- Code reviews should consider performance implications
