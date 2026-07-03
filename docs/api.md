## This is the document that lists what each public, static and private API are doing.
[it's depricated now, please don't follow it]
### search_utils
#### Here lies all the functions fdz directly relies upon
---
**find_file** : It calls *FindFirstFileExW* a WIN32 API and does two things,  
				- it first tries to find if there's any match to what it's looking for.  
					* if there is a match, it pushes directly to result vector/array.  
					* if not, then it simply moves on.  
				- It lists all the subdirectories and returns it in the end.  

*Note* : Usually the matches are rare, so when parallalizing, one can be highly relaxed about
race conditions

**list_subdirs** : This function calls the same WIN32 api as before but for this use case it only looks for subdirectories and doesn't looks at files

Why this function?
- Although the process is sequential but it can gather a list of directories in a vector,
  we can simply put them all in a giant vector and start chopping them into small batches.
  **From our observation batch size of 256 usually provides the best performance**

**concurrent_search** : This is the main entry point which connects every other services

---
#### static helper functions
This are the actual workhorses for this system. Lets do the docs on the order as they get called

**gather_bulk** : This function is reponsible for packaging the directories into a giant vectors.  
		* It achieves it by consistently calling list_subdirs() and feeding paths of directories.  
		* Once done, it checks if the returned subdirectories are in the **SKIP_LIST**(a small collection of directories to avoid, eg. node modules etc)  
		* If not then it simply adds it to the bulk vector and return in the end

**process_batch** : This calls find_file api and constantly feeds directories from the batch

**split_and_submit** :This function splits directory lists and submits them into the threadpool.  
* So, we intentionally allocate a new vector at each iteration where we copy the data to ensure threads can have locallity in the batch data they recieve
* Then move that batch of data to the threadpool.
* Threadpool calls **process_batch** to process everything

**Note** : We only synchronize the results at the end to ensure not to block any of the threads.