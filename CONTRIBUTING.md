# Contributing

Thanks for helping improve this project.

## Development standards

- Keep the code compatible with the project's current C17 target.
- Prefer small, focused functions and clear separation of responsibilities.
- Follow the existing module boundaries: `piece`, `board`, `game`, `render`, and `main`.
- Keep user-visible output clear and consistent for the terminal UI.

## Build

Use the project presets for a consistent local setup:

```bash
cmake --preset default
cmake --build --preset default
```

## Coding style

- Use 4 spaces for indentation.
- Keep functions short and readable.
- Prefer explicit conditions over clever shortcuts.
- Add comments only where they help explain intent or non-obvious logic.

## Pull requests

- Keep changes narrowly scoped and easy to review.
- Include a concise summary of what changed and why.
- Validate the project with a fresh build before submission.

## Reporting issues

When opening an issue, include:

- the exact command or steps used
- the compiler and OS version
- any error output or screenshots
- the expected versus actual behavior
