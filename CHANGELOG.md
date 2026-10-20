# Changelog

All notable changes to `codecs-php` are documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Git tags carry the plain extension version (`v0.1.0`); each release pairs it with
pinned vendored upstream codec versions recorded in the tag message,
`php_codecs.h`, and the release notes.

## [Unreleased]

### Added
- Initial extension scaffold (`config.m4`, `php_codecs.h`, `codecs.c`, `codecs.stub.php`,
  `codecs_arginfo.h`, symbol version script `codecs.map`, and PIE `composer.json` / `package.xml`
  manifests).
