# Introduction

We gratefully accepts contributions via
[pull requests](https://help.github.com/articles/about-pull-requests/).

Use the issue tracker to suggest feature requests, report bugs, and ask questions.
This is also a great way to connect with the developers of the project as well
as others who are interested in this solution.

## Changing the code-base

Generally speaking, you should fork this repository, make changes in your
own fork, and then submit a pull-request. This is often called the [Fork-and-Pull model](https://gist.github.com/Chaser324/ce0505fbed06b947d962)

The `dev` branch is the most up to date version so base your changes on this. Also make any pull requests target the dev branch.

* All contributions to this project will be released under the LICENSE.
* By submitting a pull request or filing a bug, issue, or
 feature request, you are agreeing to comply with this waiver of copyright interest.
 Details can be found in the [LICENSE](../LICENSE).
* All new code should have associated unit
tests that validate implemented features and the presence or lack of defects.
* Additionally, the code should follow any stylistic and architectural guidelines
prescribed by the project. In the absence of such guidelines, mimic the styles
and patterns in the existing code-base.

# Signing each Commit

As part of filing a pull request we ask you to sign off the
[Developer Certificate of Origin](https://developercertificate.org/) (DCO) in each commit.

(May not be valid for your project: Any Pull Request with commits that are not signed off will be reject by the
[DCO check](https://probot.github.io/apps/dco/).)

A DCO is lightweight way for a contributor to confirm that they wrote or otherwise have the right
to submit code or documentation to a project. Simply add `Signed-off-by` as shown in the example below
to indicate that you agree with the DCO.

An example signed commit message:

```
    project-utils: Add sanity checks for values of the mapping objects

    Signed-off-by: John Doe <john.doe@example.org>
```

Git has the `-s` flag that can sign a commit for you, see example below:

`$ git commit -s -m 'project-utils: Add sanity checks for values of the mapping objects'`

## Git History

In order to maintain a high software quality standard, we strongly prefer contributions to follow these rules:

- We pay more attention to the quality of commit messages. In general, we share the view on how commit messages should be written with
  [the Git project itself](https://github.com/git/git/blob/master/Documentation/SubmittingPatches):

  - [Make separate commits for logically separate changes.](https://github.com/git/git/blob/e6932248fcb41fb94a0be484050881e03c7eb298/Documentation/SubmittingPatches#L43)
    For example, pure formatting changes that do not affect software behavior usually do not belong in the same commit as
    changes to program logic.
  - [Describe your changes well.](https://github.com/git/git/blob/e6932248fcb41fb94a0be484050881e03c7eb298/Documentation/SubmittingPatches#L101)
    Do not just repeat in prose what is "obvious" from the code, but provide a rationale explaining *why* you believe
    your change is necessary.
  - [Describe your changes in the imperative.](https://github.com/git/git/blob/e6932248fcb41fb94a0be484050881e03c7eb298/Documentation/SubmittingPatches#L133)
    Instead of writing "Fixes an issue with encoding" prefer "Fix an encoding issue". Think about it like the commit
    only does something *if* it is applied. This usually results in more concise commit messages.
  - [We are picky about whitespaces.](https://github.com/git/git/blob/e6932248fcb41fb94a0be484050881e03c7eb298/Documentation/SubmittingPatches#L95)
    Trailing whitespace and duplicate blank lines are simply a superfluous annoyance, and most Git tools flag them red
    in the diff anyway.

  If you have ever wondered how a "perfect" commit message is supposed to look like, just look at basically any of
  [Jeff King's commits](https://github.com/git/git/commits?author=peff) in the Git project.

- When addressing review comments in a pull request, please fix the issue in the commit where it appears, not in a new
  commit on top of the pull request's history. While this requires force-pushing of the new iteration of your pull
  request's branch, it has several advantages:

  - Reviewers that go through (larger) pull requests commit by commit are always up-to-date with latest fixes, instead
    of coming across a commit that addresses their remarks only at the end.
  - It maintains a cleaner history without distracting commits like "Address review comments".
  - As a result, tools like [git-bisect](https://git-scm.com/docs/git-bisect) can operate in a more meaningful way.
  - Fixing up commits allows for making fixes to commit messages, which is not possible by only adding new commits.

  If you are unfamiliar with fixing up existing commits, please read about [rewriting history](https://git-scm.com/book/id/v2/Git-Tools-Rewriting-History)
  and `git rebase --interactive` in particular.

- To resolve conflicts, rebase pull request branches onto their target branch instead of merging the target branch into
  the pull request branch. This again results in a cleaner history without "criss-cross" merges.


Thank you for reading and happy contributing!
