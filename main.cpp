#include <iostream>

#include <stdio.h>  /* defines FILENAME_MAX */
#include <ctime>
#include <dirent.h>
#include <errno.h>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#include <io.h>
#define access    _access_s
#define __PATH_SEPARATOR '\\'
#define IS_WINDOWS true
#else

#include <unistd.h>

#define GetCurrentDir getcwd
#define __PATH_SEPARATOR '/'
#define IS_WINDOWS false

#endif

#include "FSW.h"
#include "git2pp.h"


inline char pathSeparator() { return __PATH_SEPARATOR; }

bool FileExists(const std::string &Filename) {
    return access(Filename.c_str(), 0) == 0;
}

bool DirExists(const std::string &Dirname) {
    DIR* dir = opendir(Dirname.c_str());
    if (dir) {
        closedir(dir);
        return true;
    } else if (ENOENT == errno) {
        return false;
    } else {
        std::cout << "Permission to open " + Dirname + " denied.";
        exit(-1);
    }
    return false;
}

typedef struct {
    bool modified = false;
} RepoStatus;

int checkstatus( const char *path, unsigned int status_flags, void *payload) {
    if (status_flags == GIT_STATUS_IGNORED) return 0;
    
    RepoStatus * status = (RepoStatus*) payload;
    status->modified = true;
    return -1;
}

void fatal(const char* msg) {
    perror(msg);
    exit(-1);
}

void initialCommit(git_signature *sig, git_repository *repo, git_index *index) {
 
  git_oid tree_id, commit_id;
  git_tree *tree;

 if (git_index_write_tree(&tree_id, index) < 0)
    fatal("Unable to write initial tree from index");

// git_index_free(index);

 if (git_tree_lookup(&tree, repo, &tree_id) < 0)
    fatal("Could not look up initial tree");

 if (git_commit_create_v(
      &commit_id, repo, "HEAD", sig, sig,
      NULL, "Initial commit", tree, 0) < 0)
    fatal("Could not create the initial commit");
    git_object *obj;
    git_object_lookup(&obj, repo, &commit_id, GIT_OBJ_ANY);
    git_reset(repo, obj, GIT_RESET_HARD, nullptr);
  
    git_tree_free(tree);
 //   git_signature_free(sig);

}

void ensureInitialCommit(std::string c) {
    git2pp::Session git;
    auto repo = git[git_repository_open](c.c_str());
    
    RepoStatus status;
    git_status_foreach(&*repo, checkstatus, &status);
    
    if (!status.modified) return;
    try {
        auto head = repo[git_repository_head]();
    } catch (...) {
        std::cout << "Creating initial commit ." << std::endl;
        auto author = git[git_signature_now]("Green system", "Green@fakemail.com");
        auto index = repo[git_repository_index]();
        initialCommit(&*author, &*repo, &*index);
    }
}

void commitAll(std::string c) {
    git2pp::Session git;
    auto repo = git[git_repository_open](c.c_str());
    
    RepoStatus status;
    git_status_foreach(&*repo, checkstatus, &status);
    
    if (!status.modified) return;
    auto head = repo[git_repository_head]();
    auto index = repo[git_repository_index]();

    git_strarray arr = {NULL, 1};
    char * all = (char *) "*\0";
    char * paths[] = {all};
    arr.strings = paths;

    git_index_update_all(&*index, nullptr, nullptr, nullptr);
    git_index_add_all(&*index, &arr, GIT_INDEX_ADD_DEFAULT, nullptr, nullptr);
    git_oid tree_oid;

    git_index_write_tree(&tree_oid, &*index);
    auto author = git[git_signature_now]("Green system", "Green@fakemail.com");
    auto tree = repo[git_tree_lookup](&tree_oid);

    git_commit * parent;
    git_oid head_oid;
    git_reference_name_to_id( &head_oid,  &*repo, "HEAD" );
    git_commit_lookup(&parent,  &*repo, &head_oid);

    const git_commit *parents[] = {
        parent
    };

    struct git_oid commit_oid;

    auto retVal = git_commit_create(
        &commit_oid,
        &*repo,
        "HEAD",
        &*author,
        &*author,
        nullptr,
        "Green commit.",
        &*tree,
        1,
        parents
    );

    if (retVal == 0) {
        git_object *obj;
        git_object_lookup(&obj, &*repo, &commit_oid, GIT_OBJ_ANY);
        git_reset(&*repo, obj, GIT_RESET_HARD, nullptr);
    } else {
        std::cout << "Green encountered an error and needs to exit. Sorry." << std::endl;
        exit(-1);
    }

}

    
int main(int argc, char **argv) {

    char currentWorkingDir[FILENAME_MAX];
    if (!GetCurrentDir(currentWorkingDir, sizeof(currentWorkingDir))) {
        return errno;
    }

    auto gitPath = std::string(currentWorkingDir) + pathSeparator() + ".git";
    if(!DirExists(gitPath)) {
        std::cout << "This is not a git repo.";
        return -1;
    }
    
    auto testExecutable = IS_WINDOWS? "test.bat" : "test";

    if (!FileExists(testExecutable)) {
        std::cout << "Green needs a "<< testExecutable << " executable in your root in order to work." << std::endl;
        return -1;
    }

    currentWorkingDir[sizeof(currentWorkingDir) - 1] = '\0'; /* not really required */
    FSW fsw(currentWorkingDir, [&](void) -> void {

        auto testPath = std::string(currentWorkingDir) + pathSeparator() + testExecutable;

        auto result = system(testPath.c_str());
        if (result == 0) {
            std::cout << "Test passed!" << std::endl;
            ensureInitialCommit(currentWorkingDir);
            commitAll(
                    currentWorkingDir
            );
        } else {
            std::cout << "Test run failed! " << result << std::endl;
        }
    });

    std::cout << "File system watcher interrupted. please check it and run again." << std::endl;

    return 0;
}