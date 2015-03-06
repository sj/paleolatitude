#!/bin/bash
#
# GitHub does not contain a full history of the Paleolatitude model because of licensing issues. The 'github' branch
# is an orphan branch based on the 'master' branch, which does not exist on GitHub.
#
# The rationale behind this set-up is:
#  - full revision history still available off-site on 'master' (and other) branch
#  - latest version publicly available on GitHub through 'github' branch
#  - pull requests on 'github' branch can be processed through GitHub
#
# Unfortunately, this means that commits on 'master' can only be merged into the 'github' branch by cherry picking those commits (a regular merge or rebase would re-introduce the full history of 'master' on the 'github' branch). The two branches operate completely independently from eachother, with no common parents.
#
# This script attempts to solve this issue by cherry picking all commits from 'master' into 'github'

set -e

git checkout --quiet master > /dev/null

diverge_rev=`git show-ref --abbrev --dereference --tags | grep 'tags/github-orphan-diverge^{}$' | cut -d " " -f 1`
master_head_rev=`git rev-parse --short HEAD`

username=`git config user.name`
email=`git config user.email`

echo "Will cherry pick the following commits from branch 'master' into 'github': "
echo "     github-orphan-diverge...$master_head_rev  (= $diverge_rev...$master_head_rev)."
echo ""
echo -n "These commits will be attributed to '$username <$email>'. Would you like to continue (y/n)? "
read -n1 response
echo 

if [ "$response" = "y" ]; then
	echo "Great. Hold on tight!"
	git checkout github
	git cherry-pick -x $diverge_rev...$master_head_rev 
else
	echo "Aborting."
	exit 1
fi

echo "You are now on branch: `git rev-parse --abbrev-ref HEAD`. Goodbye."
