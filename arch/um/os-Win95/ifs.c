#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/mm.h>
#include <linux/slab.h>

#define WSL9XFS_SUPER_MAGIC '95fs'
#define WSL9XFS_MAX_BYTES 0xffffffff
#define WSL9XFS_BLOCK_SIZE 512
#define WSL9XFS_BLOCK_SHIFT 9

struct wsl9xfs_info {
	char *host_root_path;
};

struct wsl9x_inode_info {
	int x;
};

static struct kmem_cache *wsl9xfs_inode_cache;


static const struct super_operations wsl9xfs_ops = {
	.alloc_inode	= wsl9xfs_sbops_alloc_inode,
	.free_inode	= wsl9xfs_sbops_free_inode,
	.drop_inode	= inode_just_drop,
	.evict_inode	= wsl9xfs_sbops_evict_inode,
	.statfs		= wsl9xfs_sbops_statfs,
	.show_options	= wsl9xfs_sbops_show_options,
};

struct inode* wsl9xfs_get_inode(struct super_block *sb,
				const struct inode *dir, umode_t mode, dev_t dev)
{
	struct inode* inode = new_inode(sb);
	if (!inode) {
		return NULL;
	}

	inode->i_ino = get_next_ino();
	inode_init_owner(&nop_mnt_idmap, inode, dir, mode);
	inode->i_mapping->a_ops = &ram_aops;
	mapping_set_gfp_mask(inode->i_mapping, GFP_HIGHUSER);
	mapping_set_unevictable(inode->i_mapping);
	simple_inode_init_ts(inode);
	switch (mode & S_IFMT) {
	default:
		init_special_inode(inode, mode, dev);
		break;
	case S_IFREG:
		inode->i_op = &ramfs_file_inode_operations;
		inode->i_fop = &ramfs_file_operations;
		break;
	case S_IFDIR:
		inode->i_op = &ramfs_dir_inode_operations;
		inode->i_fop = &simple_dir_operations;

		/* directory inodes start off with i_nlink == 2 (for "." entry) */
		inc_nlink(inode);
		break;
	case S_IFLNK:
		inode->i_op = &page_symlink_inode_operations;
		inode_nohighmem(inode);
		break;
	}

	return inode;
}

static int wsl9xfs_fill_super(struct super_block *sb, struct fs_context *fc)
{
	struct ramfs_fs_info *fsi = sb->s_fs_info;
	struct inode *inode;

	sb->s_maxbytes		= WSL9XFS_MAX_BYTES;
	sb->s_blocksize		= WSL9XFS_BLOCK_SIZE;
	sb->s_blocksize_bits	= WSL9XFS_BLOCK_SHIFT;
	sb->s_magic		= WSL9XFS_SUPER_MAGIC;
	sb->s_op		= &wsl9xfs_ops;
	sb->s_d_flags		= DCACHE_DONTCACHE;
	sb->s_time_gran		= 1;

	inode = ramfs_get_inode(sb, NULL, S_IFDIR | fsi->mount_opts.mode, 0);
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

static const struct fs_context_operations wsl9xfs_context_ops = {
	// .parse_monolithic = wsl9xfs_parse_monolithic,
	// .parse_param	= wsl9xfs_parse_param,
	.get_tree	= wsl9xfs_fc_get_tree,
	.free		= wsl9xfs_fc_free,
};

static int wsl9xfs_fc_get_tree(struct fs_context *fc)
{
	return get_tree_nodev(fc, wsl9xfs_fill_super);
}

static void wsl9xfs_fc_free(struct fs_context *fc)
{
	struct wsl9xfs_info *fsi = fc->s_fs_info;

	if (!fsi)
		return;

	kfree(fsi->host_root_path);
	kfree(fsi);
}

static int wsl9xfs_init_fs_context(struct fs_context *fc)
{
	struct wsl9xfs_info *fsi;

	fsi = kzalloc(sizeof(*fsi), GFP_KERNEL);
	if (!fsi)
		return -ENOMEM;

	fsi->host_root_path = kasprintf(GFP_KERNEL, "C:\\");
	if (!fsi->host_root_path) {
		kfree(fsi);
		return -ENOMEM;
	}

	fc->s_fs_info = fsi;
	fc->ops = &wsl9xfs_context_ops;
	return 0;
}

static void wsl9xfs_kill_sb(struct super_block *s)
{
	kill_anon_super(s);
	kfree(s->s_fs_info);
}

static struct file_system_type wsl9xfs_type = {
	.name			= "wsl9xfs",
	.init_fs_context	= wsl9xfs_init_fs_context,
	.kill_sb		= wsl9xfs_kill_sb,
	.fs_flags		= 0,
};

static int __init init_wsl9xfs(void)
{
	wsl9xfs_inode_cache = KMEM_CACHE(wsl9x_inode_info, 0);
	if (!wsl9xfs_inode_cache)
		return -ENOMEM;
	return register_filesystem(&wsl9xfs_type);
}

static void __exit exit_wsl9xfs(void)
{
	unregister_filesystem(&wsl9xfs_type);
	kmem_cache_destroy(wsl9xfs_inode_cache);
}

module_init(init_wsl9xfs)
module_exit(exit_wsl9xfs)
