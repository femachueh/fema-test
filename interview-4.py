from collections import Counter
arr1=[1,2,3,5,1,2,8,4,10,12]
arr2=[1,3,4]        
arr2_set = set(arr2)
nums1 = []
nums2 = [] # fema test git 3/6/2024
for i in arr1:
    if i in arr2_set: nums1.append(i)
    else: nums2.append(i)
d = Counter(nums1)
ans = []
for i in arr2:
    ans += [i] * d[i]
print (ans + sorted(nums2) )